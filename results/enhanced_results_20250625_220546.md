# Enhanced Quantitative Backtesting Results

Generated: Wed Jun 25 22:05:46 PDT 2025
System: Darwin 24.4.0
Executable: build/trading_system

## Test Configuration
- Enhanced strategies with regime detection
- Adaptive position sizing based on volatility
- Risk management with Kelly criterion
- Streaming data support for large datasets


## Enhanced_Strategies (--max-rows=500000)

**Runtime:** 105 seconds  
**Total Strategies:** 93  
**Profitable Strategies:** 6  

```text
Strategy (on Dataset)                                  Return (%)     Max DD (%)   Realized PnL     Commission     Fills      Final Equity
------------------------------------------------------------------------------------------------------------------------------------------
AdaptiveMeanRev_Aggressive_on_2024_2025                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_2024_only                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_stocks_april                  18.38          36.08           0.00          57.00        57         118375.29
AdaptiveMeanRev_Conservative_on_2024_2025                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_2024_only                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_stocks_april                39.15          57.17           0.00         164.00       164         139152.47
AdaptiveMeanRev_UltraConservative_on_2024_2025               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_2024_only               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_stocks_april          168.42         129.93           0.00        1365.00      1365         268424.22
AdvMomentum_Balanced_on_2024_2025                         -270.95         271.33     -184391.18         177.00       177        -170947.20
AdvMomentum_Balanced_on_2024_only                         -270.95         271.33     -184391.18         177.00       177        -170947.20
AdvMomentum_Balanced_on_stocks_april                        -1.98           2.95       -1283.15          27.00        27          98022.14
AdvMomentum_Conservative_on_2024_2025                        9.73          95.31      -89812.74         104.00       104         109727.05
AdvMomentum_Conservative_on_2024_only                        9.73          95.31      -89812.74         104.00       104         109727.05
AdvMomentum_Conservative_on_stocks_april                    -1.00           1.43           0.00           6.00         6          98995.93
AdvMomentum_Fast_on_2024_2025                             -571.29         469.18       -2654.61         110.00       110        -471290.62
AdvMomentum_Fast_on_2024_only                             -571.29         469.18       -2654.61         110.00       110        -471290.62
AdvMomentum_Fast_on_stocks_april                            -1.09           2.40         -82.68          78.00        78          98914.61
MACrossover_10_50_on_2024_2025                             -58.50          58.99        1701.86       57434.00     57434          41504.80
MACrossover_10_50_on_2024_only                             -58.50          58.99        1701.86       57434.00     57434          41504.80
MACrossover_10_50_on_stocks_april                           -1.64           1.80         101.67        1774.00      1774          98363.03
MACrossover_20_100_on_2024_2025                            -29.40          29.93        -217.40       28688.00     28688          70604.46
MACrossover_20_100_on_2024_only                            -29.40          29.93        -217.40       28688.00     28688          70604.46
MACrossover_20_100_on_stocks_april                          -0.95           1.15         -56.71         880.00       880          99046.73
MACrossover_3_15_on_2024_2025                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_3_15_on_2024_only                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_3_15_on_stocks_april                             0.00           0.00           0.00           0.00         0         100000.00
MACrossover_5_20_on_2024_2025                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_5_20_on_2024_only                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_5_20_on_stocks_april                             0.00           0.00           0.00           0.00         0         100000.00
ORB_30_on_stocks_april                                      -0.43           0.73         498.31        1173.00      1173          99570.49
ORB_60_on_stocks_april                                      -0.41           0.75         455.48        1103.00      1103          99589.30
Pairs_BTC_ADA_Conservative_on_2024_2025                    -75.12          75.12         470.25       75580.79     23416          24877.94
Pairs_BTC_ADA_Conservative_on_2024_only                    -75.12          75.12         470.25       75580.79     23416          24877.94
Pairs_BTC_ADA_UltraConservative_on_2024_2025               -16.52          16.52         163.41       16687.07      7824          83475.14
Pairs_BTC_ADA_UltraConservative_on_2024_only               -16.52          16.52         163.41       16687.07      7824          83475.14
Pairs_BTC_ETH_Conservative_on_2024_2025                    -23.70          23.70         -12.31       23684.00     23684          76303.07
Pairs_BTC_ETH_Conservative_on_2024_only                    -23.70          23.70         -12.31       23684.00     23684          76303.07
Pairs_BTC_ETH_UltraConservative_on_2024_2025                -8.10           8.10         -89.69        8008.00      8008          91901.91
Pairs_BTC_ETH_UltraConservative_on_2024_only                -8.10           8.10         -89.69        8008.00      8008          91901.91
Pairs_BTC_SOL_Conservative_on_2024_2025                    -22.68          22.68         735.08       23412.00     23412          77319.64
Pairs_BTC_SOL_Conservative_on_2024_only                    -22.68          22.68         735.08       23412.00     23412          77319.64
Pairs_BTC_SOL_UltraConservative_on_2024_2025                -8.06           8.06         190.90        8248.00      8248          91938.88
Pairs_BTC_SOL_UltraConservative_on_2024_only                -8.06           8.06         190.90        8248.00      8248          91938.88
Pairs_ETH_ADA_Conservative_on_2024_2025                    -75.78          75.78         440.99       76206.60     23542          24224.92
Pairs_ETH_ADA_Conservative_on_2024_only                    -75.78          75.78         440.99       76206.60     23542          24224.92
Pairs_ETH_ADA_UltraConservative_on_2024_2025               -16.24          16.24         184.15       16424.69      7702          83757.45
Pairs_ETH_ADA_UltraConservative_on_2024_only               -16.24          16.24         184.15       16424.69      7702          83757.45
Pairs_ETH_SOL_Conservative_on_2024_2025                    -23.40          23.40         625.50       24022.00     24022          76597.91
Pairs_ETH_SOL_Conservative_on_2024_only                    -23.40          23.40         625.50       24022.00     24022          76597.91
Pairs_ETH_SOL_UltraConservative_on_2024_2025                -8.02           8.02          47.30        8068.00      8068          91975.25
Pairs_ETH_SOL_UltraConservative_on_2024_only                -8.02           8.02          47.30        8068.00      8068          91975.25
Pairs_MSFT_GOOG_Conservative_on_stocks_april                -0.94           0.95         220.48        1222.00      1222          99055.30
Pairs_MSFT_GOOG_UltraConservative_on_stocks_april           -0.50           0.50         157.23         620.00       620          99502.86
Pairs_MSFT_NVDA_Conservative_on_stocks_april                -1.09           1.27         105.78        1137.00      1137          98914.77
Pairs_MSFT_NVDA_UltraConservative_on_stocks_april           -0.52           0.60          97.29         576.00       576          99481.74
Pairs_NVDA_GOOG_Conservative_on_stocks_april                -0.72           0.72         317.50        1056.00      1056          99281.74
Pairs_NVDA_GOOG_UltraConservative_on_stocks_april           -0.34           0.34         202.79         484.00       484          99657.74
Pairs_SOL_ADA_Conservative_on_2024_2025                    -76.70          76.70        1014.74       77703.34     24074          23297.88
Pairs_SOL_ADA_Conservative_on_2024_only                    -76.70          76.70        1014.74       77703.34     24074          23297.88
Pairs_SOL_ADA_UltraConservative_on_2024_2025               -16.82          16.82         311.72       17142.41      8068          83178.14
Pairs_SOL_ADA_UltraConservative_on_2024_only               -16.82          16.82         311.72       17142.41      8068          83178.14
StatArb_BTC_ETH_Aggressive_on_2024_2025                    -28.67          28.71        2965.77       31634.00     31634          71331.77
StatArb_BTC_ETH_Aggressive_on_2024_only                    -28.67          28.71        2965.77       31634.00     31634          71331.77
StatArb_BTC_ETH_Balanced_on_2024_2025                      -14.05          14.30        2740.99       16790.00     16790          85950.09
StatArb_BTC_ETH_Balanced_on_2024_only                      -14.05          14.30        2740.99       16790.00     16790          85950.09
StatArb_BTC_ETH_Conservative_on_2024_2025                   -8.68           9.20        3295.41       11980.00     11980          91315.41
StatArb_BTC_ETH_Conservative_on_2024_only                   -8.68           9.20        3295.41       11980.00     11980          91315.41
StatArb_ETH_SOL_Aggressive_on_2024_2025                    -28.07          28.43        3262.20       31328.92     31322          71933.27
StatArb_ETH_SOL_Aggressive_on_2024_only                    -28.07          28.43        3262.20       31328.92     31322          71933.27
StatArb_ETH_SOL_Balanced_on_2024_2025                      -21.13          21.23       -4382.11       16746.24     16746          78871.65
StatArb_ETH_SOL_Balanced_on_2024_only                      -21.13          21.23       -4382.11       16746.24     16746          78871.65
StatArb_ETH_SOL_Conservative_on_2024_2025                  -19.88          20.06       -7718.63       12166.23     12166          80115.47
StatArb_ETH_SOL_Conservative_on_2024_only                  -19.88          20.06       -7718.63       12166.23     12166          80115.47
StatArb_MSFT_GOOG_Aggressive_on_stocks_april                -2.64           3.99       -1332.44        1311.06      1308          97363.54
StatArb_MSFT_GOOG_Balanced_on_stocks_april                  -2.10           3.16       -1329.90         774.31       774          97895.79
StatArb_MSFT_GOOG_Conservative_on_stocks_april              -1.22           2.62        -615.06         600.00       600          98784.94
StatArb_MSFT_NVDA_Aggressive_on_stocks_april                 1.55           3.40        2877.84        1348.83      1348         101546.95
StatArb_MSFT_NVDA_Balanced_on_stocks_april                  -1.04           2.28        -394.81         646.76       646          98961.49
StatArb_MSFT_NVDA_Conservative_on_stocks_april              -1.23           4.92        -694.77         540.16       540          98765.07
StatArb_NVDA_GOOG_Aggressive_on_stocks_april                -1.39           1.88           2.07        1397.17      1360          98609.06
StatArb_NVDA_GOOG_Balanced_on_stocks_april                  -1.95           3.65       -1247.68         700.42       694          98049.38
StatArb_NVDA_GOOG_Conservative_on_stocks_april              -1.53           3.15       -1033.80         506.00       506          98468.03
VWAP_2.5_on_2024_2025                                      -53.49          78.06       68912.27        3972.00      3972          46513.77
VWAP_2.5_on_2024_only                                      -53.49          78.06       68912.27        3972.00      3972          46513.77
VWAP_2.5_on_stocks_april                                    -0.46           0.53         210.07         509.00       509          99543.87
VWAP_3.0_on_2024_2025                                      -29.91          67.39       87807.85        3604.00      3604          70087.48
VWAP_3.0_on_2024_only                                      -29.91          67.39       87807.85        3604.00      3604          70087.48
VWAP_3.0_on_stocks_april                                    -0.42           0.49         274.67         504.00       504          99578.22
VWAP_4.0_on_2024_2025                                      -78.38          92.43       48936.65        3499.00      3499          21617.09
VWAP_4.0_on_2024_only                                      -78.38          92.43       48936.65        3499.00      3499          21617.09
VWAP_4.0_on_stocks_april                                    -0.49           0.51         135.24         530.00       530          99510.50
```


## Streaming_Demo (--max-rows=100000)

**Runtime:** 18 seconds  
**Total Strategies:** 93  
**Profitable Strategies:** 8  

```text
Strategy (on Dataset)                                  Return (%)     Max DD (%)   Realized PnL     Commission     Fills      Final Equity
------------------------------------------------------------------------------------------------------------------------------------------
AdaptiveMeanRev_Aggressive_on_2024_2025                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_2024_only                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_stocks_april                  18.38          36.08           0.00          57.00        57         118375.29
AdaptiveMeanRev_Conservative_on_2024_2025                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_2024_only                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_stocks_april                39.15          57.17           0.00         164.00       164         139152.47
AdaptiveMeanRev_UltraConservative_on_2024_2025               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_2024_only               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_stocks_april          168.42         129.93           0.00        1365.00      1365         268424.22
AdvMomentum_Balanced_on_2024_2025                         -384.12         226.33      -58238.97          65.00        65        -284122.59
AdvMomentum_Balanced_on_2024_only                         -384.12         226.33      -58238.97          65.00        65        -284122.59
AdvMomentum_Balanced_on_stocks_april                        -1.98           2.95       -1283.15          27.00        27          98022.14
AdvMomentum_Conservative_on_2024_2025                     -100.01         101.07      -52951.47          19.00        19             -5.13
AdvMomentum_Conservative_on_2024_only                     -100.01         101.07      -52951.47          19.00        19             -5.13
AdvMomentum_Conservative_on_stocks_april                    -1.00           1.43           0.00           6.00         6          98995.93
AdvMomentum_Fast_on_2024_2025                             -326.42         268.59       -2654.61         110.00       110        -226422.61
AdvMomentum_Fast_on_2024_only                             -326.42         268.59       -2654.61         110.00       110        -226422.61
AdvMomentum_Fast_on_stocks_april                            -1.09           2.40         -82.68          78.00        78          98914.61
MACrossover_10_50_on_2024_2025                             -13.47          14.38       -1493.41       11660.00     11660          86528.84
MACrossover_10_50_on_2024_only                             -13.47          14.38       -1493.41       11660.00     11660          86528.84
MACrossover_10_50_on_stocks_april                           -1.64           1.80         101.67        1774.00      1774          98363.03
MACrossover_20_100_on_2024_2025                             -5.87           6.52         133.36        5876.00      5876          94133.67
MACrossover_20_100_on_2024_only                             -5.87           6.52         133.36        5876.00      5876          94133.67
MACrossover_20_100_on_stocks_april                          -0.95           1.15         -56.71         880.00       880          99046.73
MACrossover_3_15_on_2024_2025                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_3_15_on_2024_only                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_3_15_on_stocks_april                             0.00           0.00           0.00           0.00         0         100000.00
MACrossover_5_20_on_2024_2025                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_5_20_on_2024_only                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_5_20_on_stocks_april                             0.00           0.00           0.00           0.00         0         100000.00
ORB_30_on_stocks_april                                      -0.43           0.73         498.31        1173.00      1173          99570.49
ORB_60_on_stocks_april                                      -0.41           0.75         455.48        1103.00      1103          99589.30
Pairs_BTC_ADA_Conservative_on_2024_2025                    -12.96          12.96         126.08       13083.91      4774          87041.04
Pairs_BTC_ADA_Conservative_on_2024_only                    -12.96          12.96         126.08       13083.91      4774          87041.04
Pairs_BTC_ADA_UltraConservative_on_2024_2025                -2.98           2.98          37.72        3017.50      1642          97019.93
Pairs_BTC_ADA_UltraConservative_on_2024_only                -2.98           2.98          37.72        3017.50      1642          97019.93
Pairs_BTC_ETH_Conservative_on_2024_2025                     -4.80           4.80           4.48        4810.00      4810          95200.18
Pairs_BTC_ETH_Conservative_on_2024_only                     -4.80           4.80           4.48        4810.00      4810          95200.18
Pairs_BTC_ETH_UltraConservative_on_2024_2025                -1.53           1.53          -2.41        1530.00      1530          98469.16
Pairs_BTC_ETH_UltraConservative_on_2024_only                -1.53           1.53          -2.41        1530.00      1530          98469.16
Pairs_BTC_SOL_Conservative_on_2024_2025                     -4.53           4.53         195.42        4728.00      4728          95466.11
Pairs_BTC_SOL_Conservative_on_2024_only                     -4.53           4.53         195.42        4728.00      4728          95466.11
Pairs_BTC_SOL_UltraConservative_on_2024_2025                -1.69           1.69          21.87        1710.00      1710          98311.87
Pairs_BTC_SOL_UltraConservative_on_2024_only                -1.69           1.69          21.87        1710.00      1710          98311.87
Pairs_ETH_ADA_Conservative_on_2024_2025                    -12.47          12.47         180.97       12650.19      4619          87530.66
Pairs_ETH_ADA_Conservative_on_2024_only                    -12.47          12.47         180.97       12650.19      4619          87530.66
Pairs_ETH_ADA_UltraConservative_on_2024_2025                -2.75           2.75          57.99        2810.21      1532          97247.77
Pairs_ETH_ADA_UltraConservative_on_2024_only                -2.75           2.75          57.99        2810.21      1532          97247.77
Pairs_ETH_SOL_Conservative_on_2024_2025                     -4.65           4.65         327.45        4979.00      4979          95348.40
Pairs_ETH_SOL_Conservative_on_2024_only                     -4.65           4.65         327.45        4979.00      4979          95348.40
Pairs_ETH_SOL_UltraConservative_on_2024_2025                -1.67           1.67          57.75        1724.00      1724          98333.75
Pairs_ETH_SOL_UltraConservative_on_2024_only                -1.67           1.67          57.75        1724.00      1724          98333.75
Pairs_MSFT_GOOG_Conservative_on_stocks_april                -0.94           0.95         220.48        1222.00      1222          99055.30
Pairs_MSFT_GOOG_UltraConservative_on_stocks_april           -0.50           0.50         157.23         620.00       620          99502.86
Pairs_MSFT_NVDA_Conservative_on_stocks_april                -1.09           1.27         105.78        1137.00      1137          98914.77
Pairs_MSFT_NVDA_UltraConservative_on_stocks_april           -0.52           0.60          97.29         576.00       576          99481.74
Pairs_NVDA_GOOG_Conservative_on_stocks_april                -0.72           0.72         317.50        1056.00      1056          99281.74
Pairs_NVDA_GOOG_UltraConservative_on_stocks_april           -0.34           0.34         202.79         484.00       484          99657.74
Pairs_SOL_ADA_Conservative_on_2024_2025                    -13.41          13.41         389.77       13795.40      5034          86590.53
Pairs_SOL_ADA_Conservative_on_2024_only                    -13.41          13.41         389.77       13795.40      5034          86590.53
Pairs_SOL_ADA_UltraConservative_on_2024_2025                -3.01           3.01         149.90        3163.89      1714          96986.20
Pairs_SOL_ADA_UltraConservative_on_2024_only                -3.01           3.01         149.90        3163.89      1714          96986.20
StatArb_BTC_ETH_Aggressive_on_2024_2025                     -5.02           5.04        1272.55        6294.00      6294          94978.55
StatArb_BTC_ETH_Aggressive_on_2024_only                     -5.02           5.04        1272.55        6294.00      6294          94978.55
StatArb_BTC_ETH_Balanced_on_2024_2025                       -2.74           2.96         635.70        3374.00      3374          97260.11
StatArb_BTC_ETH_Balanced_on_2024_only                       -2.74           2.96         635.70        3374.00      3374          97260.11
StatArb_BTC_ETH_Conservative_on_2024_2025                   -1.93           2.29         450.07        2384.00      2384          98066.07
StatArb_BTC_ETH_Conservative_on_2024_only                   -1.93           2.29         450.07        2384.00      2384          98066.07
StatArb_ETH_SOL_Aggressive_on_2024_2025                     -3.82           4.55        2488.00        6309.80      6306          96179.22
StatArb_ETH_SOL_Aggressive_on_2024_only                     -3.82           4.55        2488.00        6309.80      6306          96179.22
StatArb_ETH_SOL_Balanced_on_2024_2025                       -2.65           3.15         764.14        3416.24      3416          97347.90
StatArb_ETH_SOL_Balanced_on_2024_only                       -2.65           3.15         764.14        3416.24      3416          97347.90
StatArb_ETH_SOL_Conservative_on_2024_2025                   -1.76           2.20         703.10        2464.23      2464          98238.87
StatArb_ETH_SOL_Conservative_on_2024_only                   -1.76           2.20         703.10        2464.23      2464          98238.87
StatArb_MSFT_GOOG_Aggressive_on_stocks_april                -2.64           3.99       -1332.44        1311.06      1308          97363.54
StatArb_MSFT_GOOG_Balanced_on_stocks_april                  -2.10           3.16       -1329.90         774.31       774          97895.79
StatArb_MSFT_GOOG_Conservative_on_stocks_april              -1.22           2.62        -615.06         600.00       600          98784.94
StatArb_MSFT_NVDA_Aggressive_on_stocks_april                 1.55           3.40        2877.84        1348.83      1348         101546.95
StatArb_MSFT_NVDA_Balanced_on_stocks_april                  -1.04           2.28        -394.81         646.76       646          98961.49
StatArb_MSFT_NVDA_Conservative_on_stocks_april              -1.23           4.92        -694.77         540.16       540          98765.07
StatArb_NVDA_GOOG_Aggressive_on_stocks_april                -1.39           1.88           2.07        1397.17      1360          98609.06
StatArb_NVDA_GOOG_Balanced_on_stocks_april                  -1.95           3.65       -1247.68         700.42       694          98049.38
StatArb_NVDA_GOOG_Conservative_on_stocks_april              -1.53           3.15       -1033.80         506.00       506          98468.03
VWAP_2.5_on_2024_2025                                        4.02          15.59        5295.43        1195.00      1195         104016.85
VWAP_2.5_on_2024_only                                        4.02          15.59        5295.43        1195.00      1195         104016.85
VWAP_2.5_on_stocks_april                                    -0.46           0.53         210.07         509.00       509          99543.87
VWAP_3.0_on_2024_2025                                       15.36          13.92       20786.75         968.00       968         115364.02
VWAP_3.0_on_2024_only                                       15.36          13.92       20786.75         968.00       968         115364.02
VWAP_3.0_on_stocks_april                                    -0.42           0.49         274.67         504.00       504          99578.22
VWAP_4.0_on_2024_2025                                      -72.93          76.40       18127.69        1163.00      1163          27066.17
VWAP_4.0_on_2024_only                                      -72.93          76.40       18127.69        1163.00      1163          27066.17
VWAP_4.0_on_stocks_april                                    -0.49           0.51         135.24         530.00       530          99510.50
```


## Full Dataset (Unlimited Rows)

**Runtime:** 124 seconds  
**Total Strategies:** 93  
**Profitable Strategies:** 4  

```text
Strategy (on Dataset)                                  Return (%)     Max DD (%)   Realized PnL     Commission     Fills      Final Equity
------------------------------------------------------------------------------------------------------------------------------------------
AdaptiveMeanRev_Aggressive_on_2024_2025                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_2024_only                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_stocks_april                  18.38          36.08           0.00          57.00        57         118375.29
AdaptiveMeanRev_Conservative_on_2024_2025                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_2024_only                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_stocks_april                39.15          57.17           0.00         164.00       164         139152.47
AdaptiveMeanRev_UltraConservative_on_2024_2025               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_2024_only               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_stocks_april          168.42         129.93           0.00        1365.00      1365         268424.22
AdvMomentum_Balanced_on_2024_2025                         -187.92         271.33     -233219.55         181.00       181         -87916.14
AdvMomentum_Balanced_on_2024_only                         -219.68         271.33     -184391.18         177.00       177        -119676.71
AdvMomentum_Balanced_on_stocks_april                        -1.98           2.95       -1283.15          27.00        27          98022.14
AdvMomentum_Conservative_on_2024_2025                     -152.69         119.05       18469.57         122.00       122         -52692.37
AdvMomentum_Conservative_on_2024_only                       -0.94          85.95      -89812.74         104.00       104          99062.65
AdvMomentum_Conservative_on_stocks_april                    -1.00           1.43           0.00           6.00         6          98995.93
AdvMomentum_Fast_on_2024_2025                             -480.79         497.64       -2654.61         110.00       110        -380788.40
AdvMomentum_Fast_on_2024_only                             -507.12         497.64       -2654.61         110.00       110        -407117.03
AdvMomentum_Fast_on_stocks_april                            -1.09           2.40         -82.68          78.00        78          98914.61
MACrossover_10_50_on_2024_2025                             -79.29          79.51        4651.38       79808.00     79808          20709.34
MACrossover_10_50_on_2024_only                             -60.50          60.94        2725.37       60466.00     60466          39497.01
MACrossover_10_50_on_stocks_april                           -1.64           1.80         101.67        1774.00      1774          98363.03
MACrossover_20_100_on_2024_2025                            -39.28          39.96        1379.54       39940.00     39940          60718.24
MACrossover_20_100_on_2024_only                            -30.97          32.02        -230.03       30250.00     30250          69028.25
MACrossover_20_100_on_stocks_april                          -0.95           1.15         -56.71         880.00       880          99046.73
MACrossover_3_15_on_2024_2025                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_3_15_on_2024_only                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_3_15_on_stocks_april                             0.00           0.00           0.00           0.00         0         100000.00
MACrossover_5_20_on_2024_2025                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_5_20_on_2024_only                                0.00           0.00           0.00           0.00         0         100000.00
MACrossover_5_20_on_stocks_april                             0.00           0.00           0.00           0.00         0         100000.00
ORB_30_on_stocks_april                                      -0.43           0.73         498.31        1173.00      1173          99570.49
ORB_60_on_stocks_april                                      -0.41           0.75         455.48        1103.00      1103          99589.30
Pairs_BTC_ADA_Conservative_on_2024_2025                    -93.53          93.53         725.03       94247.54     32353           6471.57
Pairs_BTC_ADA_Conservative_on_2024_only                    -77.39          77.39         465.24       77839.69     24651          22614.03
Pairs_BTC_ADA_UltraConservative_on_2024_2025               -20.97          20.97         105.61       21077.48     10850          79028.79
Pairs_BTC_ADA_UltraConservative_on_2024_only               -17.07          17.07         181.07       17252.22      8260          82927.65
Pairs_BTC_ETH_Conservative_on_2024_2025                    -33.25          33.25          40.94       33286.00     33286          66747.15
Pairs_BTC_ETH_Conservative_on_2024_only                    -24.97          24.97         -10.37       24960.00     24960          75027.22
Pairs_BTC_ETH_UltraConservative_on_2024_2025               -11.58          11.58         -98.76       11476.00     11476          88423.82
Pairs_BTC_ETH_UltraConservative_on_2024_only                -8.56           8.56         -86.53        8476.00      8476          91436.05
Pairs_BTC_SOL_Conservative_on_2024_2025                    -31.80          31.80         958.30       32750.00     32750          68204.41
Pairs_BTC_SOL_Conservative_on_2024_only                    -23.92          23.92         751.26       24666.00     24666          76081.92
Pairs_BTC_SOL_UltraConservative_on_2024_2025               -11.02          11.02         220.42       11230.00     11230          88984.91
Pairs_BTC_SOL_UltraConservative_on_2024_only                -8.44           8.44         183.88        8620.00      8620          91559.85
Pairs_ETH_ADA_Conservative_on_2024_2025                    -95.02          95.02         745.06       95750.69     32882           4981.56
Pairs_ETH_ADA_Conservative_on_2024_only                    -78.06          78.06         484.12       78532.75     24816          21941.32
Pairs_ETH_ADA_UltraConservative_on_2024_2025               -20.86          20.86          85.52       20939.04     10794          79140.08
Pairs_ETH_ADA_UltraConservative_on_2024_only               -16.78          16.78         199.18       16972.48      8124          83224.68
Pairs_ETH_SOL_Conservative_on_2024_2025                    -32.57          32.57         891.33       33454.00     33454          67428.96
Pairs_ETH_SOL_Conservative_on_2024_only                    -24.64          24.64         654.64       25286.00     25286          75363.05
Pairs_ETH_SOL_UltraConservative_on_2024_2025               -11.09          11.09          79.26       11168.00     11168          88909.52
Pairs_ETH_SOL_UltraConservative_on_2024_only                -8.43           8.43          45.09        8472.00      8472          91569.04
Pairs_MSFT_GOOG_Conservative_on_stocks_april                -0.94           0.95         220.48        1222.00      1222          99055.30
Pairs_MSFT_GOOG_UltraConservative_on_stocks_april           -0.50           0.50         157.23         620.00       620          99502.86
Pairs_MSFT_NVDA_Conservative_on_stocks_april                -1.09           1.27         105.78        1137.00      1137          98914.77
Pairs_MSFT_NVDA_UltraConservative_on_stocks_april           -0.52           0.60          97.29         576.00       576          99481.74
Pairs_NVDA_GOOG_Conservative_on_stocks_april                -0.72           0.72         317.50        1056.00      1056          99281.74
Pairs_NVDA_GOOG_UltraConservative_on_stocks_april           -0.34           0.34         202.79         484.00       484          99657.74
Pairs_SOL_ADA_Conservative_on_2024_2025                    -95.70          95.70        1306.26       96994.31     33298           4300.56
Pairs_SOL_ADA_Conservative_on_2024_only                    -79.06          79.06        1040.07       80083.24     25380          20943.31
Pairs_SOL_ADA_UltraConservative_on_2024_2025               -21.21          21.21         312.36       21532.30     11082          78788.44
Pairs_SOL_ADA_UltraConservative_on_2024_only               -17.37          17.37         341.86       17719.02      8512          82631.66
StatArb_BTC_ETH_Aggressive_on_2024_2025                    -39.44          39.57        4582.31       44022.00     44022          60560.31
StatArb_BTC_ETH_Aggressive_on_2024_only                    -30.31          30.32        2932.55       33246.00     33246          69686.55
StatArb_BTC_ETH_Balanced_on_2024_2025                      -19.51          19.73        4163.71       23674.00     23674          80491.03
StatArb_BTC_ETH_Balanced_on_2024_only                      -14.28          14.64        3418.36       17700.00     17700          85718.36
StatArb_BTC_ETH_Conservative_on_2024_2025                  -12.51          13.12        4388.05       16900.00     16900          87488.05
StatArb_BTC_ETH_Conservative_on_2024_only                   -8.75           9.27        3882.71       12636.00     12636          91246.71
StatArb_ETH_SOL_Aggressive_on_2024_2025                    -42.25          42.35        1600.67       43850.96     43844          57749.00
StatArb_ETH_SOL_Aggressive_on_2024_only                    -29.45          29.59        3573.00       33014.92     33008          70546.12
StatArb_ETH_SOL_Balanced_on_2024_2025                      -28.13          28.16       -4587.18       23546.24     23546          71866.58
StatArb_ETH_SOL_Balanced_on_2024_only                      -22.41          22.46       -4738.41       17668.24     17668          77589.62
StatArb_ETH_SOL_Conservative_on_2024_2025                  -22.43          22.54       -5403.38       17030.62     17030          77570.49
StatArb_ETH_SOL_Conservative_on_2024_only                  -20.45          20.62       -7661.21       12790.23     12790          79552.53
StatArb_MSFT_GOOG_Aggressive_on_stocks_april                -2.64           3.99       -1332.44        1311.06      1308          97363.54
StatArb_MSFT_GOOG_Balanced_on_stocks_april                  -2.10           3.16       -1329.90         774.31       774          97895.79
StatArb_MSFT_GOOG_Conservative_on_stocks_april              -1.22           2.62        -615.06         600.00       600          98784.94
StatArb_MSFT_NVDA_Aggressive_on_stocks_april                 1.55           3.40        2877.84        1348.83      1348         101546.95
StatArb_MSFT_NVDA_Balanced_on_stocks_april                  -1.04           2.28        -394.81         646.76       646          98961.49
StatArb_MSFT_NVDA_Conservative_on_stocks_april              -1.23           4.92        -694.77         540.16       540          98765.07
StatArb_NVDA_GOOG_Aggressive_on_stocks_april                -1.39           1.88           2.07        1397.17      1360          98609.06
StatArb_NVDA_GOOG_Balanced_on_stocks_april                  -1.95           3.65       -1247.68         700.42       694          98049.38
StatArb_NVDA_GOOG_Conservative_on_stocks_april              -1.53           3.15       -1033.80         506.00       506          98468.03
VWAP_2.5_on_2024_2025                                      -32.08          86.92       71058.25        4778.00      4778          67915.16
VWAP_2.5_on_2024_only                                      -32.15          86.78       68724.35        3974.00      3974          67853.14
VWAP_2.5_on_stocks_april                                    -0.46           0.53         210.07         509.00       509          99543.87
VWAP_3.0_on_2024_2025                                      -14.79          76.07       90186.15        4474.00      4474          85209.35
VWAP_3.0_on_2024_only                                      -10.84          75.05       87956.83        3613.00      3613          89159.62
VWAP_3.0_on_stocks_april                                    -0.42           0.49         274.67         504.00       504          99578.22
VWAP_4.0_on_2024_2025                                      -62.09         103.79       51853.90        4390.00      4390          37913.77
VWAP_4.0_on_2024_only                                      -59.02         102.25       49167.71        3508.00      3508          40983.68
VWAP_4.0_on_stocks_april                                    -0.49           0.51         135.24         530.00       530          99510.50
```

