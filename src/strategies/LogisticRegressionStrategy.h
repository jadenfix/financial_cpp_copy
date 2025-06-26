#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"      // for formatTimestampUTC
#include "core/Portfolio.h"
#include "data/PriceBar.h"

// #include <boost/circular_buffer.hpp>  // Using our own circular_buffer from Utils.h
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef LOG_TRACE
#define LOG_TRACE(fmt, ...) do{} while(0)
#endif

class LogisticRegressionStrategy : public Strategy {
private:
    //――――――――――――――――――――――――――――――――――
    // Hyper-params & data dimensions
    const size_t feature_window_ = 10;      // number of lookback bars for features
    const double  learning_rate_ = 0.01;    // SGD step
    const size_t  epochs_        = 5;       // passes over data
    const double  entry_prob_    = 0.6;     // threshold to go long
    const double  exit_prob_     = 0.4;     // threshold to go short

    // logistic model
    std::vector<double> weights_; // size = D
    double              bias_    = 0.0;

    // rolling history for live inference
    circular_buffer<PriceBar> history_{ feature_window_ + 1 };

    // thread-safety
    std::mutex mtx_;

    //――――――――――――――――――――――――――――――――――
    // Sigmoid
    static double sigmoid(double x) {
        // clamp to avoid overflow
        if (x >  20.0) x =  20.0;
        if (x < -20.0) x = -20.0;
        return 1.0/(1.0 + std::exp(-x));
    }

    // Dot product
    static double dot(const std::vector<double>& a,
                      const std::vector<double>& b) {
        double sum = 0.0;
        for (size_t i = 0; i < a.size(); ++i)
            sum += a[i]*b[i];
        return sum;
    }

    // Parse CSV file, build (X,y) for logistic regression
    void trainFromCSV(const std::string& csv_path) {
        std::ifstream in(csv_path);
        if (!in) {
            std::cout << "[ML] Warning: Cannot open CSV: " << csv_path << " - using default weights" << std::endl;
            // Initialize with small random weights if file not found
            size_t D = feature_window_*2;
            weights_.assign(D, 0.01);
            bias_ = 0.0;
            return;
        }

        // read header
        std::string line;
        std::getline(in, line);

        struct Bar { double close, volume; };
        std::vector<Bar> bars;
        bars.reserve(100000);

        // parse lines
        while (std::getline(in,line)) {
            std::stringstream ss(line);
            std::string fld;
            // open,high,low,close,volume,date,time
            for (int i=0; i<3; ++i) std::getline(ss,fld,','); // skip open,high,low
            std::getline(ss,fld,','); double close = std::stod(fld);
            std::getline(ss,fld,','); double volume = std::stod(fld);
            // skip rest
            bars.push_back({close,volume});
        }

        const size_t N = bars.size();
        if (N < feature_window_ + 2) {
            std::cout << "[ML] Warning: Not enough bars to train - using default weights" << std::endl;
            size_t D = feature_window_*2;
            weights_.assign(D, 0.01);
            bias_ = 0.0;
            return;
        }

        // build dataset
        size_t D = feature_window_*2; 
        std::vector<std::vector<double>> X;
        std::vector<int>                 y;
        X.reserve(N);
        y.reserve(N);

        for (size_t i = feature_window_; i+1 < N; ++i) {
            // features: last W returns and last W volume ratios
            std::vector<double> feats; feats.reserve(D);
            // returns
            for (size_t j=i-feature_window_+1; j<=i; ++j) {
                double ret = (bars[j].close - bars[j-1].close)
                             / std::max(bars[j-1].close, 1e-8);
                feats.push_back(ret);
            }
            // volume ratios
            double avg_vol = 0;
            for (size_t j=i-feature_window_+1; j<=i; ++j)
                avg_vol += bars[j-1].volume;
            avg_vol /= double(feature_window_);
            for (size_t j=i-feature_window_+1; j<=i; ++j)
                feats.push_back(bars[j].volume / std::max(avg_vol, 1e-8));
            // bias term absorbed in bias_

            // label: next-bar up or down
            int lbl = (bars[i+1].close > bars[i].close) ? 1 : 0;
            X.push_back(std::move(feats));
            y.push_back(lbl);
        }

        // initialize weights
        weights_.assign(D, 0.0);
        bias_ = 0.0;

        // stochastic gradient descent
        for (size_t ep=0; ep<epochs_; ++ep) {
            for (size_t i=0; i<X.size(); ++i) {
                double z = dot(weights_, X[i]) + bias_;
                double p = sigmoid(z);
                double err = p - double(y[i]);  // derivative of log-loss
                // update
                for (size_t j=0; j<D; ++j) {
                    weights_[j] -= learning_rate_ * err * X[i][j];
                }
                bias_ -= learning_rate_ * err;
            }
        }

        std::cout << "[ML] Training complete. Weights: ";
        for (double w: weights_) std::cout << w << ' ';
        std::cout << " bias="<< bias_ << std::endl;
    }

    // Build feature vector from our rolling history
    std::vector<double> computeFeatures() const {
        std::vector<double> feats; feats.reserve(weights_.size());
        // returns
        for (size_t i = 1; i < history_.size(); ++i) {
            double ret = (history_[i].Close - history_[i-1].Close)
                         / std::max(history_[i-1].Close, 1e-8);
            feats.push_back(ret);
        }
        // volumes relative
        double avg_vol = 0;
        for (size_t i = 1; i < history_.size(); ++i)
            avg_vol += history_[i-1].Volume;
        avg_vol /= double(history_.size()-1);
        for (size_t i = 1; i < history_.size(); ++i)
            feats.push_back(history_[i].Volume / std::max(avg_vol, 1e-8));
        return feats;
    }

public:
    //――――――――――――――――――――――――――――――――――
    // Pass your CSV path to train model at startup
    LogisticRegressionStrategy(const std::string& csv_path = "data/default.csv") {
        trainFromCSV(csv_path);
    }

    //――――――――――――――――――――――――――――――――――
    void handle_market_event(const MarketEvent& ev, EventQueue& queue) override {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!portfolio_) return;

        // we only trade one symbol: the primary
        // let's pick the first symbol in the snapshot
        if (ev.data.empty()) return;
        const auto& kv = *ev.data.begin();
        const std::string sym = kv.first;
        const PriceBar& bar  = kv.second;

        // update history
        history_.push_back(bar);
        if (history_.size() < feature_window_ + 1) return;

        // compute features
        auto feats = computeFeatures();
        double z = dot(weights_, feats) + bias_;
        double p = sigmoid(z);

        // decide signal
        SignalDirection want = SignalDirection::FLAT;
        if (p >  entry_prob_) want = SignalDirection::LONG;
        if (p <  exit_prob_)  want = SignalDirection::SHORT;

        static SignalDirection last = SignalDirection::FLAT;
        if (want != last) {
            LOG_TRACE(
              "ML     : {} @ {}  p={:.3f} -> {}",
              sym, formatTimestampUTC(ev.timestamp), p,
              (want==SignalDirection::LONG? "LONG":
               want==SignalDirection::SHORT?"SHORT":"FLAT")
            );
            double current_qty = portfolio_->get_position_quantity(sym);
            double target_qty = (want==SignalDirection::LONG? +1.0 : want==SignalDirection::SHORT? -1.0 : 0.0);
            double delta = target_qty - current_qty;
            if (std::abs(delta) > 1e-6) {
                send_event(std::make_shared<OrderEvent>(
                    ev.timestamp, sym,
                    OrderType::MARKET,
                    delta>0 ? OrderDirection::BUY : OrderDirection::SELL,
                    std::abs(delta)
                ), queue);
            }
            last = want;
        }
    }

    void handle_fill_event(const FillEvent&, EventQueue&) override {
        // no-op
    }

    std::string get_name() const override {
        return "MLLogisticStrategy";
    }
}; 