#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"       // for calculate_garman_klass_volatility, detect_market_regime, etc
#include "core/Portfolio.h"
#include "data/PriceBar.h"

// #include <boost/circular_buffer.hpp>  // Using our own circular_buffer from Utils.h
#include <algorithm>
#include <cmath>
#include <numeric>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#ifndef LOG_TRACE
#define LOG_TRACE(fmt, ...) do {} while(0)
#endif

class AdvancedMomentum : public Strategy {
private:
    //――――――――――――――――――――――――――――――――――――――――――――
    // 1) Centralize all “magic numbers” as constexpr
    static constexpr size_t  MIN_LOOKBACK      = 5;
    static constexpr size_t  MAX_LOOKBACK      = 200;
    static constexpr double  MIN_MOM_THRESHOLD = 0.1;
    static constexpr double  MAX_MOM_THRESHOLD = 10.0;
    static constexpr double  MIN_VOL_FILTER    = 0.1;
    static constexpr double  MAX_VOL_FILTER    = 5.0;
    static constexpr double  MIN_POSITION_RISK = 0.0;
    static constexpr double  MAX_POSITION_RISK = 0.5;
    static constexpr size_t  MAX_HISTORY       = 200;   // bars & returns
    static constexpr size_t  MAX_TRADES        = 50;    // for Kelly stats
    static constexpr double  EPS               = 1e-8;  // for divide-by-zero guards

    //――――――――――――――――――――――――――――――――――――――――――――
    // 2) User-tunable parameters (clamped)
    size_t lookback_period_;
    double momentum_threshold_;
    double volatility_filter_;
    double max_position_risk_;
    size_t warmup_bars_;

    //――――――――――――――――――――――――――――――――――――――――――――
    // 3) Per-symbol state with fixed-size ring buffers
    struct SymbolState {
        circular_buffer<PriceBar> bars{MAX_HISTORY};
        circular_buffer<double>    rets{MAX_HISTORY};
        circular_buffer<double>    closes{MAX_HISTORY};
        circular_buffer<double>    trade_rets{MAX_TRADES};

        // Kelly stats
        double win_rate = 0.5,
               avg_win  = 0.01,
               avg_loss = -0.01;

        // Current position tracking
        SignalDirection current_signal = SignalDirection::FLAT;
        double entry_price = 0.0;
        std::chrono::system_clock::time_point last_trade_time;

        // Regime & vol
        MarketRegime current_regime{};
        double current_vol = 0.0;
    };
    std::map<std::string, SymbolState> states_;
    mutable std::mutex state_mutex_;

    //――――――――――――――――――――――――――――――――――――――――――――
    // 4) Static helpers for testable, self-contained logic

    // 4a) Compute a Sharpe-style momentum score
    static double calcMomentum(const circular_buffer<double>& rets,
                               size_t lookback)
    {
        if (rets.size() < lookback) return 0.0;

        // mean + stddev via STL
        auto start = rets.last(lookback);
        auto end = rets.end();
        double mean = std::accumulate(start, end, 0.0) / lookback;

        double var = std::inner_product(
            start, end, start, 0.0,
            std::plus<>(),
            [mean](double x, double y){ 
                return (x - mean) * (y - mean); 
            }
        );
        double sd = std::sqrt(var / (lookback - 1));
        return (sd < EPS ? 0.0 : mean / sd);
    }

    // 4b) Update win_rate, avg_win, avg_loss via Kelly history
    void updatePerformance(SymbolState& s, double trade_ret) {
        s.trade_rets.push_back(trade_ret);

        if (s.trade_rets.size() < 10) return;
        int wins=0, losses=0;
        double sum_w=0, sum_l=0;
        for (double r : s.trade_rets) {
            if (r > 0) { wins++; sum_w += r; }
            else if (r < 0) { losses++; sum_l += r; }
        }
        if (wins+losses > 0) {
            s.win_rate = double(wins)/(wins+losses);
            s.avg_win  = (wins>0 ? sum_w/wins : s.avg_win);
            s.avg_loss = (losses>0? sum_l/losses : s.avg_loss);
        }
    }

public:
    AdvancedMomentum(int lookback = 20,
                     double mom_thresh = 1.5,
                     double vol_filter = 0.8,
                     double max_risk = 0.03)
    {
        lookback_period_      = std::clamp<size_t>(lookback, MIN_LOOKBACK, MAX_LOOKBACK);
        momentum_threshold_   = std::clamp(mom_thresh,  MIN_MOM_THRESHOLD, MAX_MOM_THRESHOLD);
        volatility_filter_    = std::clamp(vol_filter, MIN_VOL_FILTER,    MAX_VOL_FILTER);
        max_position_risk_    = std::clamp(max_risk,    MIN_POSITION_RISK, MAX_POSITION_RISK);
        warmup_bars_          = lookback_period_ + 10;
    }

    //――――――――――――――――――――――――――――――――――――――――――――
    void handle_market_event(const MarketEvent& ev, EventQueue& q) override {
        std::lock_guard lock(state_mutex_);
        if (!portfolio_) return;

        for (auto const& [sym,bar] : ev.data) {
            auto &st = states_.try_emplace(sym, SymbolState{}).first->second;

            // 5) Early warm-up exit
            if (st.bars.size() < warmup_bars_) {
                st.bars.push_back(bar);
                st.closes.push_back(bar.Close);
                if (st.bars.size()>1) {
                    double prev = std::prev(st.bars.end(),2)->Close;
                    double r = (prev<EPS?0.0:std::log(bar.Close/prev));
                    st.rets.push_back(r);
                }
                continue;
            }

            // 6) Rolling history update
            st.bars.push_back(bar);
            st.closes.push_back(bar.Close);
            {
                double prev = std::prev(st.bars.end(),2)->Close;
                double r = (prev<EPS?0.0:std::log(bar.Close/prev));
                st.rets.push_back(r);
            }

            // 7) Compute analytics
            st.current_vol = calculate_garman_klass_volatility(
                                  {st.bars.begin(), st.bars.end()}, lookback_period_);
            st.current_regime = detect_market_regime(
                                  {st.closes.begin(), st.closes.end()}, lookback_period_);
            double mom_score = calcMomentum(st.rets, lookback_period_);

            // 8) Regime-aware signal logic
            SignalDirection want = SignalDirection::FLAT;
            if (st.current_regime.regime == MarketRegime::TRENDING &&
                st.current_regime.strength > 0.3)
            {
                if (mom_score >  momentum_threshold_ &&
                    st.current_vol < volatility_filter_) want = SignalDirection::LONG;
                else if (mom_score < -momentum_threshold_ &&
                         st.current_vol < volatility_filter_) want = SignalDirection::SHORT;
            }
            else if (st.current_regime.regime == MarketRegime::MEAN_REVERTING &&
                     st.current_regime.strength > 0.5)
            {
                if (mom_score >  2.0*momentum_threshold_) want = SignalDirection::SHORT;
                if (mom_score < -2.0*momentum_threshold_) want = SignalDirection::LONG;
            }

            // 9) Position sizing & execution
            if (want != st.current_signal) {
                double cur_pos = portfolio_->get_position_quantity(sym);
                // — close old trade, update perf stats
                if (std::abs(cur_pos) > EPS && st.entry_price > 0) {
                    double ret = (cur_pos>0)
                                  ? std::log(bar.Close / st.entry_price)
                                  : std::log(st.entry_price / bar.Close);
                    updatePerformance(st, ret);
                }

                // — Kelly + vol sizing (via your Utils)
                double kelly = calculate_kelly_position_size(
                                  st.win_rate, st.avg_win, st.avg_loss,
                                  max_position_risk_);
                double equity = portfolio_->get_total_equity();
                double dollar_risk = equity * kelly;
                double shares = calculate_volatility_adjusted_shares(
                                  dollar_risk, bar.Close, st.current_vol, 2.0);

                // — regime tilt
                if (st.current_regime.regime == MarketRegime::TRENDING) shares *= 1.2;
                else if (st.current_regime.regime == MarketRegime::MEAN_REVERTING) shares *= 0.6;

                double target = (want==SignalDirection::LONG ? +shares : -shares);
                target = std::clamp(target, -20.0, 20.0);
                double delta  = target - cur_pos;

                if (std::abs(delta) > 0.1) {
                    auto dir = delta>0 ? OrderDirection::BUY : OrderDirection::SELL;
                    double qty = std::abs(delta);
                    LOG_TRACE("AM: {} t={} score={} vol={} regime={} str={} kelly={} qty={} {}",
                              sym,
                              formatTimestampUTC(ev.timestamp),
                              mom_score, st.current_vol,
                              (int)st.current_regime.regime,
                              st.current_regime.strength,
                              kelly, qty,
                              (dir==OrderDirection::BUY?"BUY":"SELL")
                    );
                    send_event(std::make_shared<OrderEvent>(
                                   ev.timestamp, sym,
                                   OrderType::MARKET, dir, qty),
                               q);
                    st.current_signal = want;
                    st.entry_price    = bar.Close;
                    st.last_trade_time= ev.timestamp;
                }
            }
        }
    }

    void handle_fill_event(const FillEvent& /*ev*/, EventQueue& /*q*/) override {
        // no‐op or your existing logic
    }
};