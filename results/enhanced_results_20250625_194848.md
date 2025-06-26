# Enhanced Quantitative Backtesting Results

Generated: Wed Jun 25 19:48:48 PDT 2025
System: Darwin 24.4.0
Executable: build/trading_system

## Test Configuration
- Enhanced strategies with regime detection
- Adaptive position sizing based on volatility
- Risk management with Kelly criterion
- Streaming data support for large datasets


## Enhanced_Strategies (--max-rows=500000)

**Runtime:** 117 seconds  
**Total Strategies:** 93  
**Profitable Strategies:** 4  

```text
Strategy (on Dataset)                                  Return (%)     Max DD (%)   Realized PnL     Commission     Fills      Final Equity
------------------------------------------------------------------------------------------------------------------------------------------
AdaptiveMeanRev_Aggressive_on_2024_2025                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_2024_only                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_stocks_april                   0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_2024_2025                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_2024_only                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_stocks_april                 0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_2024_2025               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_2024_only               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_stocks_april            0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Balanced_on_2024_2025                            0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Balanced_on_2024_only                            0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Balanced_on_stocks_april                         0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Conservative_on_2024_2025                        0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Conservative_on_2024_only                        0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Conservative_on_stocks_april                     0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Fast_on_2024_2025                                0.25           0.13         260.30          14.00        14         100246.30
AdvMomentum_Fast_on_2024_only                                0.25           0.13         260.30          14.00        14         100246.30
AdvMomentum_Fast_on_stocks_april                             0.00           0.00           2.56           2.00         2         100000.56
MACrossover_10_50_on_2024_2025                             -57.98          58.47        1365.35       56820.00     56820          42017.26
MACrossover_10_50_on_2024_only                             -57.98          58.47        1365.35       56820.00     56820          42017.26
MACrossover_10_50_on_stocks_april                           -1.97           2.13        -325.97        1658.00      1658          98029.99
MACrossover_20_100_on_2024_2025                            -29.04          29.57           3.31       28554.00     28554          70964.65
MACrossover_20_100_on_2024_only                            -29.04          29.57           3.31       28554.00     28554          70964.65
MACrossover_20_100_on_stocks_april                          -1.09           1.26        -239.24         854.00       854          98914.24
MACrossover_3_15_on_2024_2025                             -102.45         102.79       18550.78      105633.00    105633          -2448.94
MACrossover_3_15_on_2024_only                             -102.45         102.79       18550.78      105633.00    105633          -2448.94
MACrossover_3_15_on_stocks_april                            -5.47           5.59        -442.58        5038.00      5038          94526.54
MACrossover_5_20_on_2024_2025                             -103.30         103.77        5729.64       99787.00     99787          -3303.95
MACrossover_5_20_on_2024_only                             -103.30         103.77        5729.64       99787.00     99787          -3303.95
MACrossover_5_20_on_stocks_april                            -3.81           3.99        -115.11        3692.00      3692          96190.61
ORB_30_on_stocks_april                                      -0.59           0.96        -646.84         129.00       129          99413.15
ORB_60_on_stocks_april                                      -0.54           0.90        -614.96         107.00       107          99461.43
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
StatArb_BTC_ETH_Aggressive_on_2024_2025                    -85.57          86.62      -46885.98       30858.00     30858          14426.55
StatArb_BTC_ETH_Aggressive_on_2024_only                    -85.57          86.62      -46885.98       30858.00     30858          14426.55
StatArb_BTC_ETH_Balanced_on_2024_2025                        9.51          70.65      -16973.93       18770.00     18770         109514.06
StatArb_BTC_ETH_Balanced_on_2024_only                        9.51          70.65      -16973.93       18770.00     18770         109514.06
StatArb_BTC_ETH_Conservative_on_2024_2025                   -8.23          44.48       25356.98        8504.00      8504          91766.93
StatArb_BTC_ETH_Conservative_on_2024_only                   -8.23          44.48       25356.98        8504.00      8504          91766.93
StatArb_ETH_SOL_Aggressive_on_2024_2025                    -34.17          41.63       25894.99       73815.00     73815          65829.57
StatArb_ETH_SOL_Aggressive_on_2024_only                    -34.17          41.63       25894.99       73815.00     73815          65829.57
StatArb_ETH_SOL_Balanced_on_2024_2025                      -27.35          34.25       -1188.33       25934.00     25934          72649.74
StatArb_ETH_SOL_Balanced_on_2024_only                      -27.35          34.25       -1188.33       25934.00     25934          72649.74
StatArb_ETH_SOL_Conservative_on_2024_2025                  -38.92          43.68      -40403.22       11128.00     11128          61078.85
StatArb_ETH_SOL_Conservative_on_2024_only                  -38.92          43.68      -40403.22       11128.00     11128          61078.85
StatArb_MSFT_GOOG_Aggressive_on_stocks_april                -2.40           3.44        -221.33        2394.00      2394          97604.76
StatArb_MSFT_GOOG_Balanced_on_stocks_april                  -1.71           2.58        -710.48         900.00       900          98287.00
StatArb_MSFT_GOOG_Conservative_on_stocks_april              -0.77           1.50         175.03         536.00       536          99231.92
StatArb_MSFT_NVDA_Aggressive_on_stocks_april                -0.89           3.26        2108.12        2590.00      2590          99111.36
StatArb_MSFT_NVDA_Balanced_on_stocks_april                  -1.77           2.44        -889.69         965.00       965          98233.47
StatArb_MSFT_NVDA_Conservative_on_stocks_april              -0.37           2.19        -653.21         658.00       658          99630.28
StatArb_NVDA_GOOG_Aggressive_on_stocks_april                -3.00           3.01        -947.66        2372.00      2372          96996.15
StatArb_NVDA_GOOG_Balanced_on_stocks_april                  -1.38           1.86        -444.65         950.00       950          98616.04
StatArb_NVDA_GOOG_Conservative_on_stocks_april              -1.39           1.71        -811.29         500.00       500          98613.44
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

**Runtime:** 22 seconds  
**Total Strategies:** 93  
**Profitable Strategies:** 6  

```text
Strategy (on Dataset)                                  Return (%)     Max DD (%)   Realized PnL     Commission     Fills      Final Equity
------------------------------------------------------------------------------------------------------------------------------------------
AdaptiveMeanRev_Aggressive_on_2024_2025                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_2024_only                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_stocks_april                   0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_2024_2025                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_2024_only                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_stocks_april                 0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_2024_2025               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_2024_only               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_stocks_april            0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Balanced_on_2024_2025                            0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Balanced_on_2024_only                            0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Balanced_on_stocks_april                         0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Conservative_on_2024_2025                        0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Conservative_on_2024_only                        0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Conservative_on_stocks_april                     0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Fast_on_2024_2025                               -0.11           0.11        -102.91           4.00         4          99893.09
AdvMomentum_Fast_on_2024_only                               -0.11           0.11        -102.91           4.00         4          99893.09
AdvMomentum_Fast_on_stocks_april                             0.00           0.00           2.56           2.00         2         100000.56
MACrossover_10_50_on_2024_2025                             -13.44          14.33       -1580.52       11542.00     11542          86560.48
MACrossover_10_50_on_2024_only                             -13.44          14.33       -1580.52       11542.00     11542          86560.48
MACrossover_10_50_on_stocks_april                           -1.97           2.13        -325.97        1658.00      1658          98029.99
MACrossover_20_100_on_2024_2025                             -5.86           6.50         110.30        5842.00      5842          94143.31
MACrossover_20_100_on_2024_only                             -5.86           6.50         110.30        5842.00      5842          94143.31
MACrossover_20_100_on_stocks_april                          -1.09           1.26        -239.24         854.00       854          98914.24
MACrossover_3_15_on_2024_2025                              -36.30          36.30        2967.67       35979.00     35979          63704.46
MACrossover_3_15_on_2024_only                              -36.30          36.30        2967.67       35979.00     35979          63704.46
MACrossover_3_15_on_stocks_april                            -5.47           5.59        -442.58        5038.00      5038          94526.54
MACrossover_5_20_on_2024_2025                              -26.22          26.27         358.58       25501.00     25501          73776.42
MACrossover_5_20_on_2024_only                              -26.22          26.27         358.58       25501.00     25501          73776.42
MACrossover_5_20_on_stocks_april                            -3.81           3.99        -115.11        3692.00      3692          96190.61
ORB_30_on_stocks_april                                      -0.59           0.96        -646.84         129.00       129          99413.15
ORB_60_on_stocks_april                                      -0.54           0.90        -614.96         107.00       107          99461.43
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
StatArb_BTC_ETH_Aggressive_on_2024_2025                    -43.83          49.48      -24366.41       13334.00     13334          56174.43
StatArb_BTC_ETH_Aggressive_on_2024_only                    -43.83          49.48      -24366.41       13334.00     13334          56174.43
StatArb_BTC_ETH_Balanced_on_2024_2025                        5.63          34.92        4784.22        5152.00      5152         105629.60
StatArb_BTC_ETH_Balanced_on_2024_only                        5.63          34.92        4784.22        5152.00      5152         105629.60
StatArb_BTC_ETH_Conservative_on_2024_2025                   -2.03          34.51       16126.15        2417.00      2417          97970.77
StatArb_BTC_ETH_Conservative_on_2024_only                   -2.03          34.51       16126.15        2417.00      2417          97970.77
StatArb_ETH_SOL_Aggressive_on_2024_2025                    -15.26          17.59       -1888.00       14396.00     14396          84740.97
StatArb_ETH_SOL_Aggressive_on_2024_only                    -15.26          17.59       -1888.00       14396.00     14396          84740.97
StatArb_ETH_SOL_Balanced_on_2024_2025                      -14.23          15.34       -5380.69        5180.00      5180          85768.36
StatArb_ETH_SOL_Balanced_on_2024_only                      -14.23          15.34       -5380.69        5180.00      5180          85768.36
StatArb_ETH_SOL_Conservative_on_2024_2025                   -3.49           7.65        1834.24        2168.00      2168          96514.32
StatArb_ETH_SOL_Conservative_on_2024_only                   -3.49           7.65        1834.24        2168.00      2168          96514.32
StatArb_MSFT_GOOG_Aggressive_on_stocks_april                -2.40           3.44        -221.33        2394.00      2394          97604.76
StatArb_MSFT_GOOG_Balanced_on_stocks_april                  -1.71           2.58        -710.48         900.00       900          98287.00
StatArb_MSFT_GOOG_Conservative_on_stocks_april              -0.77           1.50         175.03         536.00       536          99231.92
StatArb_MSFT_NVDA_Aggressive_on_stocks_april                -0.89           3.26        2108.12        2590.00      2590          99111.36
StatArb_MSFT_NVDA_Balanced_on_stocks_april                  -1.77           2.44        -889.69         965.00       965          98233.47
StatArb_MSFT_NVDA_Conservative_on_stocks_april              -0.37           2.19        -653.21         658.00       658          99630.28
StatArb_NVDA_GOOG_Aggressive_on_stocks_april                -3.00           3.01        -947.66        2372.00      2372          96996.15
StatArb_NVDA_GOOG_Balanced_on_stocks_april                  -1.38           1.86        -444.65         950.00       950          98616.04
StatArb_NVDA_GOOG_Conservative_on_stocks_april              -1.39           1.71        -811.29         500.00       500          98613.44
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

**Runtime:** 141 seconds  
**Total Strategies:** 93  
**Profitable Strategies:** 5  

```text
Strategy (on Dataset)                                  Return (%)     Max DD (%)   Realized PnL     Commission     Fills      Final Equity
------------------------------------------------------------------------------------------------------------------------------------------
AdaptiveMeanRev_Aggressive_on_2024_2025                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_2024_only                      0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Aggressive_on_stocks_april                   0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_2024_2025                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_2024_only                    0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_Conservative_on_stocks_april                 0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_2024_2025               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_2024_only               0.00           0.00           0.00           0.00         0         100000.00
AdaptiveMeanRev_UltraConservative_on_stocks_april            0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Balanced_on_2024_2025                            0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Balanced_on_2024_only                            0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Balanced_on_stocks_april                         0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Conservative_on_2024_2025                        0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Conservative_on_2024_only                        0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Conservative_on_stocks_april                     0.00           0.00           0.00           0.00         0         100000.00
AdvMomentum_Fast_on_2024_2025                                0.24           0.13         261.89          18.00        18         100243.89
AdvMomentum_Fast_on_2024_only                                0.25           0.13         261.90          16.00        16         100245.90
AdvMomentum_Fast_on_stocks_april                             0.00           0.00           2.56           2.00         2         100000.56
MACrossover_10_50_on_2024_2025                             -78.78          79.00        3884.75       78947.00     78947          21222.41
MACrossover_10_50_on_2024_only                             -59.99          60.41        2363.17       59822.00     59822          40013.42
MACrossover_10_50_on_stocks_april                           -1.97           2.13        -325.97        1658.00      1658          98029.99
MACrossover_20_100_on_2024_2025                            -39.03          39.70        1438.06       39750.00     39750          60969.27
MACrossover_20_100_on_2024_only                            -30.64          31.67         -41.50       30110.00     30110          69357.48
MACrossover_20_100_on_stocks_april                          -1.09           1.26        -239.24         854.00       854          98914.24
MACrossover_3_15_on_2024_2025                              -99.79         103.26       21211.66      107652.00    107652            208.10
MACrossover_3_15_on_2024_only                             -101.12         103.26       18550.78      105633.00    105633          -1122.24
MACrossover_3_15_on_stocks_april                            -5.47           5.59        -442.58        5038.00      5038          94526.54
MACrossover_5_20_on_2024_2025                              -99.49         104.10        6848.81      100885.00    100885            508.76
MACrossover_5_20_on_2024_only                             -101.61         104.10        5729.64       99787.00     99787          -1608.78
MACrossover_5_20_on_stocks_april                            -3.81           3.99        -115.11        3692.00      3692          96190.61
ORB_30_on_stocks_april                                      -0.59           0.96        -646.84         129.00       129          99413.15
ORB_60_on_stocks_april                                      -0.54           0.90        -614.96         107.00       107          99461.43
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
StatArb_BTC_ETH_Aggressive_on_2024_2025                    -75.29          87.59      -32052.35       34656.00     34656          24712.02
StatArb_BTC_ETH_Aggressive_on_2024_only                    -83.48          87.59      -44617.45       31030.00     31030          16523.08
StatArb_BTC_ETH_Balanced_on_2024_2025                       77.36          46.32       57457.62       23890.00     23890         177355.75
StatArb_BTC_ETH_Balanced_on_2024_only                       23.19          70.65       -1909.76       19394.00     19394         123192.84
StatArb_BTC_ETH_Conservative_on_2024_2025                   14.65          44.48       53255.95        9978.00      9978         114648.31
StatArb_BTC_ETH_Conservative_on_2024_only                  -20.76          44.48       13035.17        8604.00      8604          79244.81
StatArb_ETH_SOL_Aggressive_on_2024_2025                    -58.70          61.01       33949.57      104556.00    104556          41296.82
StatArb_ETH_SOL_Aggressive_on_2024_only                    -41.84          43.64       29858.41       78114.00     78114          58162.32
StatArb_ETH_SOL_Balanced_on_2024_2025                      -50.17          58.03       -2972.22       36038.00     36038          49826.03
StatArb_ETH_SOL_Balanced_on_2024_only                      -31.59          34.25       -3289.66       27423.00     27423          68412.57
StatArb_ETH_SOL_Conservative_on_2024_2025                  -21.29          43.68      -21525.05       15224.00     15224          78711.50
StatArb_ETH_SOL_Conservative_on_2024_only                  -30.90          43.68      -34022.71       11712.00     11712          69100.30
StatArb_MSFT_GOOG_Aggressive_on_stocks_april                -2.40           3.44        -221.33        2394.00      2394          97604.76
StatArb_MSFT_GOOG_Balanced_on_stocks_april                  -1.71           2.58        -710.48         900.00       900          98287.00
StatArb_MSFT_GOOG_Conservative_on_stocks_april              -0.77           1.50         175.03         536.00       536          99231.92
StatArb_MSFT_NVDA_Aggressive_on_stocks_april                -0.89           3.26        2108.12        2590.00      2590          99111.36
StatArb_MSFT_NVDA_Balanced_on_stocks_april                  -1.77           2.44        -889.69         965.00       965          98233.47
StatArb_MSFT_NVDA_Conservative_on_stocks_april              -0.37           2.19        -653.21         658.00       658          99630.28
StatArb_NVDA_GOOG_Aggressive_on_stocks_april                -3.00           3.01        -947.66        2372.00      2372          96996.15
StatArb_NVDA_GOOG_Balanced_on_stocks_april                  -1.38           1.86        -444.65         950.00       950          98616.04
StatArb_NVDA_GOOG_Conservative_on_stocks_april              -1.39           1.71        -811.29         500.00       500          98613.44
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

