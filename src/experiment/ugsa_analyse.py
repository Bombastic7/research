#!/bin/python



import os
import json
import csv


if __name__ == "__main__":
	
	s = json.loads(
R'{"search log": "n expd gend dups reopnd hbf cachedsol\n0 14945 24952 9872 0 0 0\n1 14937 24918 9836 0 1 0\n2 14888 24848 9784 0 1 0\n3 14529 24324 9375 0 1 0\n4 14487 24232 9325 0 1 0\n5 13915 23417 8728 0 1.5 0\n6 13158 22162 7988 0 2 0\n7 14794 24684 9674 0 2 0\n8 13564 22866 8377 0 2 0\n9 20788 35004 19921 8982 2.5 0\n10 14288 23939 9137 0 2.5 0\n11 14701 24557 9579 0 2 0\n12 14702 24594 9600 0 2 0\n13 14581 24407 9459 0 2 0\n14 14106 23480 8820 0 1.75 0\n15 12252 20829 7074 0 1.75 0\n16 14255 23821 8997 0 1.6 0\n17 12031 20311 6764 0 1.6 0\n18 14147 23735 8925 0 1.6 0\n19 13262 22155 7931 0 1.5 0\n20 10434 17668 5346 0 1.5 0\n21 9739 16362 4720 0 1.42857 0\n22 7564 12939 3066 0 1.375 0\n23 10231 17413 5211 0 1.375 0\n24 9735 16342 4703 0 1.33333 0\n25 4573 7956 1402 0 1.33333 0\n26 10022 16749 4954 0 1.33333 0\n27 3857 6411 1085 0 1.3 0\n28 6872 11607 2671 0 1.3 0\n29 2701 4525 650 0 1.27273 0\n30 1821 3044 376 0 1.25 0\n31 3403 5729 944 0 1.25 0\n32 841 1473 134 0 1.23077 0\n33 1846 3203 434 0 1.23077 0\n34 1030 1746 181 0 1.23077 0\n35 435 755 68 0 1.21429 0\n36 1142 1912 188 0 1.21429 0\n37 330 580 46 0 1.2 0\n38 462 798 79 0 1.1875 0\n39 321 544 50 0 1.1875 0\n40 341 600 51 0 1.17647 0\n41 314 546 49 0 1.17647 0\n42 150 253 16 0 1.17647 0\n43 66 111 5 0 1.17647 0\n44 209 356 31 0 1.17647 0\n45 37 66 4 0 1.16667 0\n46 71 128 10 0 1.16667 0\n47 24 41 4 0 1.16667 0\n48 13 22 2 0 1.22222 0\n49 24 41 2 0 1.27778 0\n50 8 15 1 0 1.27778 0\n51 3 7 0 0 1.33333 0\n52 14 24 2 0 1.33333 0\n53 14 24 2 0 1.33333 0\n54 0 0 0 0 1.38889 0\n55 4 9 0 0 1.38889 0\n56 1 3 0 0 1.16667 0\n57 5 10 0 0 1.16667 0\n58 11 18 2 0 1.16667 0\n59 16 25 0 0 1.16667 0\n60 18 32 4 0 1.15789 0\n61 17 30 2 0 1.15789 0\n62 19 35 2 0 1.15789 0\n63 17 30 2 0 1.15789 0\n64 47 82 4 0 1.15789 0\n65 75 137 12 0 1.15789 0\n66 39 70 4 0 1.15789 0\n67 87 156 12 0 1.15789 0\n68 19 34 3 0 1.15789 0\n69 54 95 5 0 1.15789 0\n70 13 26 1 0 1.15789 0\n71 29 48 2 0 1.15789 0\n72 11 19 2 0 1.15789 0\n73 11 19 2 0 1.15789 0\n74 17 30 0 0 1.15789 0\n75 19 33 4 0 1.15789 0\n76 16 29 2 0 1.15789 0\n77 18 34 2 0 1.15789 0\n78 16 29 2 0 1.15789 0\n79 18 33 0 0 1.15789 0\n80 22 39 0 0 1.15789 0\n81 54 93 3 0 1.15789 0\n82 14 24 2 0 1.15789 0\n83 8 15 1 0 1.15789 0\n84 18 32 4 0 1.15789 0\n85 3 7 0 0 1.15789 0\n86 13 22 2 0 1.15789 0\n87 14 24 2 0 1.15789 0\n88 0 0 0 0 1.15789 0\n89 4 9 0 0 1.15789 0\n90 1 3 0 0 1.15789 0\n91 14 24 2 0 1.15789 0\n92 19 35 2 0 1.15789 0\n93 8 15 1 0 1.15789 0\n94 3 7 0 0 1.15789 0\n95 14 24 2 0 1.15789 0\n96 13 22 2 0 1.15789 0\n97 0 0 0 0 1.15789 0\n98 4 9 0 0 1.15789 0\n99 1 3 0 0 1.15789 0\n100 48 82 9 0 1.15789 0\n101 48 81 4 0 1.15789 0\n102 424 713 73 0 1.15789 0\n103 89 154 12 0 1.15789 0\n104 64 109 9 0 1.15789 0\n105 117 204 18 0 1.15789 0\n106 178 298 27 0 1.15789 0\n107 116 200 19 0 1.15789 0\n108 132 229 27 0 1.15789 0\n109 0 0 0 0 -1 1\n110 631 1061 117 0 1.15789 0\n111 604 1021 112 0 1.15789 0\n112 571 968 108 0 1.15789 0\n113 563 961 110 0 1.15789 0\n114 5 10 0 0 1.15 0\n115 11 18 2 0 1.15 0\n116 0 0 0 0 -1 1\n117 0 0 0 0 -1 1\n118 16 25 0 0 1.15 0\n119 0 0 0 0 -1 1\n120 25 46 0 0 1.15 0\n121 27 46 2 0 1.15 0\n122 19 35 2 0 1.15 0\n123 17 30 2 0 1.15 0\n124 24 41 4 0 1.15 0\n125 24 41 2 0 1.15 0\n126 18 32 4 0 1.15 0\n127 0 0 0 0 -1 1\n128 0 0 0 0 -1 1\n129 40 69 0 0 1.15 0\n130 40 69 0 0 1.15 0\n131 34 61 0 0 1.15 0\n132 41 73 8 0 1.15 0\n133 39 68 4 0 1.15 0\n134 34 61 1 0 1.15 0\n135 11 19 2 0 1.15 0\n136 19 33 4 0 1.15 0\n137 13 26 1 0 1.15 0\n138 11 19 2 0 1.15 0\n139 0 0 0 0 -1 1\n140 0 0 0 0 -1 1\n141 0 0 0 0 -1 1\n142 18 34 2 0 1.15 0\n143 0 0 0 0 -1 1\n144 0 0 0 0 -1 1\n145 785 1305 152 0 1.15 0\n146 496 843 87 0 1.15 0\n147 894 1548 183 0 1.15 0\n148 90 157 10 0 1.14286 0\n149 61 107 5 0 1.14286 0\n150 67 114 12 0 1.14286 0\n151 85 143 14 0 1.14286 0\n152 79 138 7 0 1.14286 0\n153 124 203 10 0 1.14286 0\n154 149 250 16 0 1.14286 0\n155 147 249 21 0 1.14286 0\n156 149 251 22 0 1.14286 0\n157 127 218 21 0 1.14286 0\n158 238 409 39 0 1.14286 0\n159 152 269 26 0 1.14286 0\n160 734 1304 143 0 1.14286 0\n161 871 1513 165 0 1.14286 0\n162 754 1321 141 0 1.14286 0\n163 740 1301 149 0 1.14286 0\n164 1186 2098 251 0 1.14286 0\n165 993 1748 211 0 1.14286 0\n166 17 30 0 0 1.14286 0\n167 0 0 0 0 -1 1\n168 18 33 0 0 1.14286 0\n169 22 39 0 0 1.14286 0\n170 0 0 0 0 -1 1\n171 0 0 0 0 -1 1\n172 0 0 0 0 -1 1\n173 16 29 2 0 1.14286 0\n174 0 0 0 0 -1 1\n175 19 34 3 0 1.14286 0\n176 29 48 2 0 1.14286 0\n177 0 0 0 0 -1 1\n178 0 0 0 0 -1 1\n179 38 69 4 0 1.14286 0\n180 0 0 0 0 -1 1\n181 47 82 4 0 1.14286 0\n182 0 0 0 0 -1 1\n183 53 88 3 0 1.14286 0\n184 56 96 5 0 1.14286 0\n185 56 96 5 0 1.14286 0\n186 0 0 0 0 -1 1\n187 37 66 4 0 1.14286 0\n188 0 0 0 0 -1 1\n189 0 0 0 0 -1 1\n190 0 0 0 0 -1 1\n191 75 137 12 0 1.14286 0\n192 71 128 10 0 1.14286 0\n193 0 0 0 0 -1 1\n194 39 70 4 0 1.14286 0\n195 87 156 12 0 1.14286 0\n196 66 111 5 0 1.14286 0\n197 54 95 5 0 1.14286 0\n198 54 93 3 0 1.14286 0\n199 316 539 55 0 1.14286 0\n200 219 385 34 0 1.14286 0\n201 0 0 0 0 -1 1\n202 0 0 0 0 -1 1\n203 0 0 0 0 -1 1\n204 0 0 0 0 -1 1\n205 0 0 0 0 -1 1\n206 0 0 0 0 -1 1\n207 0 0 0 0 -1 1\n208 0 0 0 0 -1 1\n209 520 890 87 0 1.14286 0\n210 1342 2250 257 0 1.14286 0\n211 412 693 66 0 1.14286 0\n212 520 892 87 0 1.14286 0\n213 324 557 60 0 1.14286 0\n214 326 569 57 0 1.14286 0\n215 282 478 48 0 1.2381 0\n216 541 921 102 0 1.2381 0\n217 447 746 81 0 1.2381 0\n218 535 901 96 0 1.2381 0\n219 315 558 57 0 1.2381 0\n220 1039 1769 196 0 1.2381 0\n221 2173 3715 528 0 1.2381 0\n222 55 96 11 0 1.2381 0\n223 87 145 14 0 1.2381 0\n224 74 137 8 0 1.2381 0\n225 49 85 5 0 1.2381 0\n226 141 242 14 0 1.2381 0\n227 143 244 8 0 1.2381 0\n228 177 292 19 0 1.2381 0\n229 180 301 19 0 1.2381 0\n230 0 0 0 0 -1 1\n231 0 0 0 0 -1 1\n232 0 0 0 0 -1 1\n233 0 0 0 0 -1 1\n234 0 0 0 0 -1 1\n235 0 0 0 0 -1 1\n236 60 106 3 0 1.2381 0\n237 63 112 7 0 1.2381 0\n238 70 121 2 0 1.2381 0\n239 70 119 1 0 1.2381 0\n240 60 108 3 0 1.2381 0\n241 68 121 6 0 1.2381 0\n242 54 95 6 0 1.2381 0\n243 65 116 10 0 1.2381 0\n244 87 150 3 0 1.22727 0\n245 86 149 6 0 1.22727 0\n246 0 0 0 0 -1 1\n247 0 0 0 0 -1 1\n248 120 199 10 0 1.22727 0\n249 68 117 6 0 1.22727 0\n250 0 0 0 0 -1 1\n251 0 0 0 0 -1 1\n252 0 0 0 0 -1 1\n253 0 0 0 0 -1 1\n254 53 96 1 0 1.22727 0\n255 53 94 1 0 1.22727 0\n256 53 94 1 0 1.22727 0\n257 53 96 1 0 1.22727 0\n258 0 0 0 0 -1 1\n259 0 0 0 0 -1 1\n260 104 178 17 0 1.22727 0\n261 150 253 16 0 1.22727 0\n262 0 0 0 0 -1 1\n263 0 0 0 0 -1 1\n264 0 0 0 0 -1 1\n265 122 210 14 0 1.22727 0\n266 87 158 12 0 1.22727 0\n267 101 172 15 0 1.22727 0\n268 83 150 12 0 1.22727 0\n269 152 251 14 0 1.22727 0\n270 101 184 11 0 1.22727 0\n271 101 184 11 0 1.22727 0\n272 183 322 22 0 1.22727 0\n273 202 351 24 0 1.22727 0\n274 178 308 26 0 1.22727 0\n275 176 294 27 0 1.22727 0\n276 0 0 0 0 -1 1\n277 0 0 0 0 -1 1\n278 157 283 23 0 1.22727 0\n279 136 242 21 0 1.22727 0\n280 196 331 32 0 1.22727 0\n281 168 300 27 0 1.22727 0\n282 0 0 0 0 -1 1\n283 777 1317 144 0 1.22727 0\n284 952 1618 188 0 1.22727 0\n285 921 1587 182 0 1.22727 0\n286 935 1592 180 0 1.22727 0\n287 415 717 78 0 1.22727 0\n288 532 897 90 0 1.22727 0\n289 969 1667 190 0 1.22727 0\n290 1310 2258 256 0 1.22727 0\n291 977 1699 192 0 1.22727 0\n292 566 969 102 0 1.22727 0\n293 1280 2163 286 0 1.22727 0\n294 1423 2438 325 0 1.22727 0\n295 1142 1950 219 0 1.22727 0\n296 1202 2122 254 0 1.22727 0\n297 1992 3427 504 0 1.22727 0\n298 2233 3802 542 0 1.22727 0\n299 1882 3261 446 0 1.22727 0\n300 1943 3327 477 0 1.22727 0\n301 0 0 0 0 -1 1\n302 0 0 0 0 -1 1\n303 0 0 0 0 -1 1\n304 0 0 0 0 -1 1\n305 0 0 0 0 -1 1\n306 0 0 0 0 -1 1\n307 0 0 0 0 -1 1\n308 0 0 0 0 -1 1\n309 0 0 0 0 -1 1\n310 0 0 0 0 -1 1\n311 0 0 0 0 -1 1\n312 0 0 0 0 -1 1\n313 0 0 0 0 -1 1\n314 0 0 0 0 -1 1\n315 0 0 0 0 -1 1\n316 0 0 0 0 -1 1\n317 0 0 0 0 -1 1\n318 0 0 0 0 -1 1\n319 0 0 0 0 -1 1\n320 0 0 0 0 -1 1\n321 0 0 0 0 -1 1\n322 0 0 0 0 -1 1\n323 0 0 0 0 -1 1\n324 0 0 0 0 -1 1\n325 76 141 11 0 1.21739 0\n326 124 212 14 0 1.21739 0\n327 114 189 18 0 1.21739 0\n328 114 191 17 0 1.21739 0\n329 100 172 16 0 1.21739 0\n330 170 279 23 0 1.21739 0\n331 170 285 21 0 1.21739 0\n332 169 284 21 0 1.21739 0\n333 0 0 0 0 -1 1\n334 0 0 0 0 -1 1\n335 388 661 63 0 1.21739 0\n336 495 851 74 0 1.21739 0\n337 336 581 54 0 1.21739 0\n338 0 0 0 0 -1 1\n339 0 0 0 0 -1 1\n340 0 0 0 0 -1 1\n341 0 0 0 0 -1 1\n342 0 0 0 0 -1 1\n343 0 0 0 0 -1 1\n344 0 0 0 0 -1 1\n345 0 0 0 0 -1 1\n346 0 0 0 0 -1 1\n347 0 0 0 0 -1 1\n348 0 0 0 0 -1 1\n349 0 0 0 0 -1 1\n350 0 0 0 0 -1 1\n351 0 0 0 0 -1 1\n352 0 0 0 0 -1 1\n353 0 0 0 0 -1 1\n354 0 0 0 0 -1 1\n355 0 0 0 0 -1 1\n356 0 0 0 0 -1 1\n357 0 0 0 0 -1 1\n358 201 352 23 0 1.20833 0\n359 190 339 22 0 1.20833 0\n360 237 404 28 0 1.20833 0\n361 179 308 24 0 1.20833 0\n362 262 435 33 0 1.20833 0\n363 253 418 32 0 1.20833 0\n364 253 420 38 0 1.20833 0\n365 254 421 36 0 1.20833 0\n366 461 817 76 0 1.20833 0\n367 774 1353 145 0 1.20833 0\n368 697 1173 119 0 1.20833 0\n369 833 1436 160 0 1.20833 0\n370 1797 3128 398 0 1.20833 0\n371 0 0 0 0 -1 1\n372 0 0 0 0 -1 1\n373 0 0 0 0 -1 1\n374 0 0 0 0 -1 1\n375 0 0 0 0 -1 1\n376 0 0 0 0 -1 1\n377 0 0 0 0 -1 1\n378 0 0 0 0 -1 1\n379 0 0 0 0 -1 1\n380 0 0 0 0 -1 1\n381 0 0 0 0 -1 1\n382 0 0 0 0 -1 1\n383 0 0 0 0 -1 1\n384 0 0 0 0 -1 1\n385 0 0 0 0 -1 1\n386 0 0 0 0 -1 1\n387 108 192 10 0 1.2 0\n388 0 0 0 0 -1 1\n389 117 206 15 0 1.2 0\n390 75 135 11 0 1.2 0\n391 117 196 18 0 1.2 0\n392 127 219 11 0 1.2 0\n393 119 206 8 0 1.2 0\n394 113 201 10 0 1.2 0\n395 101 182 14 0 1.2 0\n396 116 207 7 0 1.2 0\n397 113 198 9 0 1.2 0\n398 136 235 9 0 1.2 0\n399 136 233 10 0 1.2 0\n400 135 234 10 0 1.2 0\n401 0 0 0 0 -1 1\n402 103 182 13 0 1.2 0\n403 113 200 9 0 1.2 0\n404 109 193 12 0 1.2 0\n405 0 0 0 0 -1 1\n406 0 0 0 0 -1 1\n407 0 0 0 0 -1 1\n408 115 200 8 0 1.2 0\n409 107 180 16 0 1.2 0\n410 0 0 0 0 -1 1\n411 0 0 0 0 -1 1\n412 0 0 0 0 -1 1\n413 250 411 38 0 1.2 0\n414 115 199 19 0 1.2 0\n415 151 269 23 0 1.2 0\n416 165 291 20 0 1.2 0\n417 202 340 28 0 1.2 0\n418 91 161 12 0 1.2 0\n419 176 316 26 0 1.2 0\n420 176 316 24 0 1.2 0\n421 162 278 21 0 1.2 0\n422 109 196 21 0 1.2 0\n423 158 287 18 0 1.2 0\n424 154 281 21 0 1.2 0\n425 121 222 17 0 1.2 0\n426 0 0 0 0 -1 1\n427 0 0 0 0 -1 1\n428 0 0 0 0 -1 1\n429 0 0 0 0 -1 1\n430 0 0 0 0 -1 1\n431 0 0 0 0 -1 1\n432 0 0 0 0 -1 1\n433 557 989 107 0 1.2 0\n434 616 1050 101 0 1.2 0\n435 586 986 107 0 1.2 0\n436 808 1356 153 0 1.2 0\n437 675 1181 130 0 1.2 0\n438 745 1319 144 0 1.2 0\n439 0 0 0 0 -1 1\n440 0 0 0 0 -1 1\n441 0 0 0 0 -1 1\n442 0 0 0 0 -1 1\n443 0 0 0 0 -1 1\n444 0 0 0 0 -1 1\n445 0 0 0 0 -1 1\n446 0 0 0 0 -1 1\n447 0 0 0 0 -1 1\n448 0 0 0 0 -1 1\n449 0 0 0 0 -1 1\n450 0 0 0 0 -1 1\n451 0 0 0 0 -1 1\n452 0 0 0 0 -1 1\n453 0 0 0 0 -1 1\n454 0 0 0 0 -1 1\n455 0 0 0 0 -1 1\n456 0 0 0 0 -1 1\n457 0 0 0 0 -1 1\n458 0 0 0 0 -1 1\n459 0 0 0 0 -1 1\n460 0 0 0 0 -1 1\n461 0 0 0 0 -1 1\n462 0 0 0 0 -1 1\n463 0 0 0 0 -1 1\n464 0 0 0 0 -1 1\n465 0 0 0 0 -1 1\n466 0 0 0 0 -1 1\n467 0 0 0 0 -1 1\n468 0 0 0 0 -1 1\n469 0 0 0 0 -1 1\n470 0 0 0 0 -1 1\n471 0 0 0 0 -1 1\n472 0 0 0 0 -1 1\n473 0 0 0 0 -1 1\n474 0 0 0 0 -1 1\n475 0 0 0 0 -1 1\n476 0 0 0 0 -1 1\n477 0 0 0 0 -1 1\n478 0 0 0 0 -1 1\n479 0 0 0 0 -1 1\n480 0 0 0 0 -1 1\n481 0 0 0 0 -1 1\n482 0 0 0 0 -1 1\n483 0 0 0 0 -1 1\n484 0 0 0 0 -1 1\n485 0 0 0 0 -1 1\n486 0 0 0 0 -1 1\n487 154 262 20 0 1.2 0\n488 108 195 21 0 1.2 0\n489 209 356 31 0 1.2 0\n490 321 544 50 0 1.2 0\n491 0 0 0 0 -1 1\n492 0 0 0 0 -1 1\n493 124 210 21 0 1.2 0\n494 158 282 25 0 1.2 0\n495 187 328 26 0 1.2 0\n496 193 325 23 0 1.2 0\n497 219 367 32 0 1.2 0\n498 96 170 13 0 1.2 0\n499 199 332 31 0 1.2 0\n500 0 0 0 0 -1 1\n501 209 353 28 0 1.2 0\n502 209 355 30 0 1.2 0\n503 186 327 19 0 1.2 0\n504 187 330 24 0 1.2 0\n505 348 611 53 0 1.2 0\n506 365 631 55 0 1.2 0\n507 304 539 45 0 1.2 0\n508 402 672 76 0 1.2 0\n509 311 522 49 0 1.2 0\n510 341 595 48 0 1.2 0\n511 307 546 51 0 1.2 0\n512 304 536 44 0 1.2 0\n513 339 583 44 0 1.2 0\n514 1211 2052 241 0 1.2 0\n515 746 1279 123 0 1.2 0\n516 839 1404 153 0 1.2 0\n517 870 1450 153 0 1.2 0\n518 593 1007 92 0 1.2 0\n519 1243 2169 251 0 1.2 0\n520 1281 2160 275 0 1.2 0\n521 1794 2994 410 0 1.2 0\n522 454 777 79 0 1.2 0\n523 0 0 0 0 -1 1\n524 616 1026 124 0 1.2 0\n525 0 0 0 0 -1 1\n526 0 0 0 0 -1 1\n527 0 0 0 0 -1 1\n528 0 0 0 0 -1 1\n529 0 0 0 0 -1 1\n530 0 0 0 0 -1 1\n531 0 0 0 0 -1 1\n532 0 0 0 0 -1 1\n533 0 0 0 0 -1 1\n534 0 0 0 0 -1 1\n535 0 0 0 0 -1 1\n536 0 0 0 0 -1 1\n537 0 0 0 0 -1 1\n538 1 3 0 0 1.2 0\n539 0 0 0 0 -1 1\n540 0 0 0 0 1.2 0\n541 0 0 0 0 -1 1\n542 3 7 0 0 1.2 0\n543 8 15 1 0 1.2 0\n544 4 9 0 0 1.2 0\n545 0 0 0 0 -1 1\n546 0 0 0 0 -1 1\n547 0 0 0 0 -1 1\n548 0 0 0 0 -1 1\n549 0 0 0 0 -1 1\n550 0 0 0 0 -1 1\n551 0 0 0 0 -1 1\n552 0 0 0 0 -1 1\n553 0 0 0 0 -1 1\n554 14 24 2 0 1.2 0\n555 13 22 2 0 1.2 0\n556 14 24 2 0 1.2 0\n557 0 0 0 0 -1 1\n558 0 0 0 0 -1 1\n559 0 0 0 0 -1 1\n560 0 0 0 0 -1 1\n561 0 0 0 0 -1 1\n562 0 0 0 0 -1 1\n563 0 0 0 0 -1 1\n564 0 0 0 0 -1 1\n565 0 0 0 0 -1 1\n566 0 0 0 0 -1 1\n567 0 0 0 0 -1 1\n568 0 0 0 0 -1 1\n569 0 0 0 0 -1 1\n570 0 0 0 0 -1 1\n571 0 0 0 0 -1 1\n572 0 0 0 0 -1 1\n573 0 0 0 0 -1 1\n574 0 0 0 0 -1 1\n575 0 0 0 0 -1 1\n576 0 0 0 0 -1 1\n577 0 0 0 0 -1 1\n578 0 0 0 0 -1 1\n579 0 0 0 0 -1 1\n580 0 0 0 0 -1 1\n581 0 0 0 0 -1 1\n582 0 0 0 0 -1 1\n583 0 0 0 0 -1 1\n584 0 0 0 0 -1 1\n585 0 0 0 0 -1 1\n586 0 0 0 0 -1 1\n587 0 0 0 0 -1 1\n588 0 0 0 0 -1 1\n589 0 0 0 0 -1 1\n590 0 0 0 0 -1 1\n591 0 0 0 0 -1 1\n592 0 0 0 0 -1 1\n593 0 0 0 0 -1 1\n594 0 0 0 0 -1 1\n595 0 0 0 0 -1 1\n596 0 0 0 0 -1 1\n597 0 0 0 0 -1 1\n598 0 0 0 0 -1 1\n599 0 0 0 0 -1 1\n600 0 0 0 0 -1 1\n601 0 0 0 0 -1 1\n602 0 0 0 0 -1 1\n603 0 0 0 0 -1 1\n604 0 0 0 0 -1 1\n605 0 0 0 0 -1 1\n606 0 0 0 0 -1 1\n607 0 0 0 0 -1 1\n608 0 0 0 0 -1 1\n609 0 0 0 0 -1 1\n610 606 1024 109 0 1.2 0\n611 555 947 95 0 1.2 0\n612 520 884 88 0 1.2 0\n613 503 862 88 0 1.2 0\n614 0 0 0 0 -1 1\n615 0 0 0 0 -1 1\n616 0 0 0 0 -1 1\n617 0 0 0 0 -1 1\n618 0 0 0 0 -1 1\n619 0 0 0 0 -1 1\n620 0 0 0 0 -1 1\n621 0 0 0 0 -1 1\n622 0 0 0 0 -1 1\n623 0 0 0 0 -1 1\n624 0 0 0 0 -1 1\n625 0 0 0 0 -1 1\n626 0 0 0 0 -1 1\n627 0 0 0 0 -1 1\n628 0 0 0 0 -1 1\n629 831 1434 166 0 1.2 0\n630 1344 2246 254 0 1.2 0\n631 1883 3142 394 0 1.2 0\n632 2297 3875 570 0 1.2 0\n633 2501 4225 581 0 1.2 0\n634 1720 3015 362 0 1.2 0\n635 2470 4151 639 0 1.2 0\n636 0 0 0 0 -1 1\n637 0 0 0 0 -1 1\n638 0 0 0 0 -1 1\n639 0 0 0 0 -1 1\n640 0 0 0 0 -1 1\n641 0 0 0 0 -1 1\n642 0 0 0 0 -1 1\n643 0 0 0 0 -1 1\n644 0 0 0 0 -1 1\n645 0 0 0 0 -1 1\n646 0 0 0 0 -1 1\n647 0 0 0 0 -1 1\n648 0 0 0 0 -1 1\n649 0 0 0 0 -1 1\n650 0 0 0 0 -1 1\n651 0 0 0 0 -1 1\n652 0 0 0 0 -1 1\n653 0 0 0 0 -1 1\n654 0 0 0 0 -1 1\n655 0 0 0 0 -1 1\n656 0 0 0 0 -1 1\n657 0 0 0 0 -1 1\n658 0 0 0 0 -1 1\n659 0 0 0 0 -1 1\n660 0 0 0 0 -1 1\n661 0 0 0 0 -1 1\n662 0 0 0 0 -1 1\n663 0 0 0 0 -1 1\n664 0 0 0 0 -1 1\n665 0 0 0 0 -1 1\n666 0 0 0 0 -1 1\n667 0 0 0 0 -1 1\n668 0 0 0 0 -1 1\n669 0 0 0 0 -1 1\n670 0 0 0 0 -1 1\n671 0 0 0 0 -1 1\n672 0 0 0 0 -1 1\n673 0 0 0 0 -1 1\n674 0 0 0 0 -1 1\n675 0 0 0 0 -1 1\n676 0 0 0 0 -1 1\n677 0 0 0 0 -1 1\n678 0 0 0 0 -1 1\n679 169 308 21 0 1.19231 0\n680 240 405 32 0 1.19231 0\n681 193 333 26 0 1.19231 0\n682 100 176 16 0 1.19231 0\n683 148 268 17 0 1.19231 0\n684 204 351 24 0 1.19231 0\n685 0 0 0 0 -1 1\n686 0 0 0 0 -1 1\n687 211 356 31 0 1.19231 0\n688 194 329 32 0 1.19231 0\n689 246 413 29 0 1.19231 0\n690 242 405 32 0 1.19231 0\n691 122 218 17 0 1.19231 0\n692 109 191 18 0 1.19231 0\n693 171 308 17 0 1.19231 0\n694 170 307 19 0 1.19231 0\n695 0 0 0 0 -1 1\n696 0 0 0 0 -1 1\n697 0 0 0 0 -1 1\n698 0 0 0 0 -1 1\n699 0 0 0 0 -1 1\n700 0 0 0 0 -1 1\n701 0 0 0 0 -1 1\n702 0 0 0 0 -1 1\n703 0 0 0 0 -1 1\n704 258 443 30 0 1.19231 0\n705 224 375 27 0 1.19231 0\n706 252 435 33 0 1.19231 0\n707 223 378 27 0 1.19231 0\n708 229 392 28 0 1.19231 0\n709 177 316 20 0 1.19231 0\n710 210 360 34 0 1.19231 0\n711 184 315 26 0 1.19231 0\n712 0 0 0 0 -1 1\n713 158 259 30 0 1.19231 0\n714 257 442 33 0 1.19231 0\n715 0 0 0 0 -1 1\n716 204 353 22 0 1.19231 0\n717 263 448 26 0 1.19231 0\n718 0 0 0 0 -1 1\n719 0 0 0 0 -1 1\n720 165 304 18 0 1.19231 0\n721 248 421 30 0 1.19231 0\n722 166 293 23 0 1.19231 0\n723 199 345 32 0 1.19231 0\n724 148 268 21 0 1.19231 0\n725 261 444 36 0 1.19231 0\n726 0 0 0 0 -1 1\n727 0 0 0 0 -1 1\n728 206 353 22 0 1.19231 0\n729 265 454 30 0 1.19231 0\n730 0 0 0 0 -1 1\n731 0 0 0 0 -1 1\n732 0 0 0 0 -1 1\n733 0 0 0 0 -1 1\n734 412 696 62 0 1.19231 0\n735 314 519 51 0 1.19231 0\n736 204 354 34 0 1.19231 0\n737 257 439 39 0 1.19231 0\n738 245 425 37 0 1.19231 0\n739 347 591 48 0 1.19231 0\n740 298 521 47 0 1.19231 0\n741 396 667 67 0 1.19231 0\n742 239 418 39 0 1.19231 0\n743 409 699 72 0 1.19231 0\n744 432 759 73 0 1.19231 0\n745 667 1116 103 0 1.19231 0\n746 524 894 97 0 1.19231 0\n747 1221 2081 280 0 1.19231 0\n748 1211 2065 263 0 1.19231 0\n749 933 1618 184 0 1.19231 0\n750 1217 2043 258 0 1.19231 0\n751 1140 1948 234 0 1.19231 0\n752 1021 1731 192 0 1.19231 0\n753 1096 1893 227 0 1.19231 0\n754 992 1720 197 0 1.19231 0\n755 536 949 95 0 1.19231 0\n756 974 1628 180 0 1.19231 0\n757 879 1517 151 0 1.19231 0\n758 900 1562 167 0 1.19231 0\n759 1087 1841 212 0 1.19231 0\n760 0 0 0 0 -1 1\n761 969 1655 189 0 1.19231 0\n762 0 0 0 0 -1 1\n763 2879 4800 777 0 1.19231 0\n764 2861 4872 776 0 1.19231 0\n765 0 0 0 0 -1 1\n766 0 0 0 0 -1 1\n767 0 0 0 0 -1 1\n768 0 0 0 0 -1 1\n769 0 0 0 0 -1 1\n770 0 0 0 0 -1 1\n771 0 0 0 0 -1 1\n772 0 0 0 0 -1 1\n773 0 0 0 0 -1 1\n774 0 0 0 0 -1 1\n775 0 0 0 0 -1 1\n776 0 0 0 0 -1 1\n777 0 0 0 0 -1 1\n778 0 0 0 0 -1 1\n779 0 0 0 0 -1 1\n780 0 0 0 0 -1 1\n781 0 0 0 0 -1 1\n782 0 0 0 0 -1 1\n783 0 0 0 0 -1 1\n784 0 0 0 0 -1 1\n785 0 0 0 0 -1 1\n786 0 0 0 0 -1 1\n787 0 0 0 0 -1 1\n788 0 0 0 0 -1 1\n789 0 0 0 0 -1 1\n790 0 0 0 0 -1 1\n791 0 0 0 0 -1 1\n792 0 0 0 0 -1 1\n793 0 0 0 0 -1 1\n794 0 0 0 0 -1 1\n795 0 0 0 0 -1 1\n796 0 0 0 0 -1 1\n797 0 0 0 0 -1 1\n798 0 0 0 0 -1 1\n799 0 0 0 0 -1 1\n800 0 0 0 0 -1 1\n801 0 0 0 0 -1 1\n802 0 0 0 0 -1 1\n803 0 0 0 0 -1 1\n804 0 0 0 0 -1 1\n805 0 0 0 0 -1 1\n806 323 532 52 0 1.18519 0\n807 265 441 45 0 1.18519 0\n808 263 453 44 0 1.18519 0\n809 330 585 59 0 1.18519 0\n810 328 571 55 0 1.18519 0\n811 357 612 59 0 1.18519 0\n812 273 459 47 0 1.18519 0\n813 0 0 0 0 -1 1\n814 0 0 0 0 -1 1\n815 0 0 0 0 -1 1\n816 0 0 0 0 -1 1\n817 0 0 0 0 -1 1\n818 0 0 0 0 -1 1\n819 0 0 0 0 -1 1\n820 0 0 0 0 -1 1\n821 0 0 0 0 -1 1\n822 0 0 0 0 -1 1\n823 0 0 0 0 -1 1\n824 0 0 0 0 -1 1\n825 0 0 0 0 -1 1\n826 0 0 0 0 -1 1\n827 0 0 0 0 -1 1\n828 0 0 0 0 -1 1\n829 0 0 0 0 -1 1\n830 0 0 0 0 -1 1\n831 0 0 0 0 -1 1\n832 0 0 0 0 -1 1\n833 0 0 0 0 -1 1\n834 0 0 0 0 -1 1\n835 0 0 0 0 -1 1\n836 0 0 0 0 -1 1\n837 0 0 0 0 -1 1\n838 0 0 0 0 -1 1\n839 0 0 0 0 -1 1\n840 0 0 0 0 -1 1\n841 0 0 0 0 -1 1\n842 0 0 0 0 -1 1\n843 0 0 0 0 -1 1\n844 0 0 0 0 -1 1\n"}')
 
	with open("ugsa_out.txt", "w") as f:
		f.write(s["search log"])
