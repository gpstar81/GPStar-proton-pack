/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Dustin Grau <dustin.grau@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

// Compress first via https://processing.compress-or-die.com/svg-process

const char EQUIP_svg[] PROGMEM = R"=====(
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="0 0 2950 3840">
  <path d="M238 2910c0 18-8 33-18 33s-18-15-18-33c0-18 8-33 18-33s18 15 18 33Z" fill="#fff" stroke="#000" stroke-linejoin="round" stroke-width="4"/>
  <path d="M222 2868h35v85h-35Zm1875 42c0 18-8 33-18 33s-18-15-18-33c0-18 8-33 18-33s18 15 18 33Z" fill="#fff" stroke="#000" stroke-linejoin="round" stroke-width="4"/>
  <path d="M2076 2953h-35v-85h35Z" fill="#fff" stroke="#000" stroke-linejoin="round" stroke-width="4"/>
  <path d="M1923 2085h86v1294h-86Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1869 3127h158v218H1940s4-42-47-42h-37ZM371 3379h-86V2085h86Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M426 3127H268v218h87s-5-42 47-42H440Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M415 3454 270 3283c-84-100-164-256-135-396 10-40 23-80 40-119l12-30c20-52 24-93 25-147l1-132h29l-2 133a407 407 0 01-39 187c-16 37-30 75-39 114-26 130 53 281 130 371l144 171Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1670 150h163s106-5 168 71c62 77 60 153 60 153v240s26 12 27 27v502l-140 1754H325v-823l133-236v-262s2-86-108-88H26V723h152V186h800s-7-64 55-66h603s34-2 34 26ZM392 3457 528 3340l126 145-136 118Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1346 961h455v1352h-455Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m1390 1266 78-220H984v796H838v366H1380l10-942Zm122-685h340v473h-340Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1040 2186h215v1638H1040Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m435 2453 140-162L1860 3405l-140 162Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m330 2674 89-196 1547 707-90 195Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m290 2914 31-212 1683 242-30 212Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m321 3156-30-212 1683-242 30 212Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m1490 3715-206 60-450-1533 206-60Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m1697 3586-180 116-862-1341 181-116Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M418 3380 330 3184l1450-662 89 196Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m575 3567-140-162L1638 2360l140 163Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m778 3706-180-116 859-1336 180 116Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m1010 3775-205-60 448-1530 206 62Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1983 2929a836 836 0 11-1671 0 836 836 0 01 1671 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1148 2093a835 835 0 10 798 587l-1-3-379-1280c-8-30-22-60-40-87a167 167 0 00-305 95l-1 690Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1866 2929a719 719 0 11-1437 0 719 719 0 01 1437 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1801 2929a654 654 0 11-1307 0 654 654 0 01 1307 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1605 3510a197 197 0 11-393 0 197 197 0 01 393 0Zm113-1563a248 248 0 11-497 0 248 248 0 01 497 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1703 1947a234 234 0 11-467 0 234 234 0 01 467 0Zm-661 698a175 175 0 11-350 0 175 175 0 01 350 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M952 2645a85 85 0 11-170 0 85 85 0 01 170 0Zm655 0a175 175 0 11-350 0 175 175 0 01 350 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1517 2645a85 85 0 11-170 0 85 85 0 01 170 0Zm-423 478a164 164 0 11-329 0 164 164 0 01 329 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1015 3123a86 86 0 11-171 0 86 86 0 01 171 0Zm519 0a164 164 0 11-330 0 164 164 0 01 330 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1455 3123a86 86 0 11-172 0 86 86 0 01 172 0Zm155 376a210 210 0 11-420 0 210 210 0 01 420 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1595 3499a195 195 0 11-390 0 195 195 0 01 390 0ZM1830 581h199c16 0 28 13 28 29v509c0 16-12 29-28 29h-200a29 29 0 01-28-30V610a30 30 0 01 29-29Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1867 642h208v74h-208Zm0 185h208v75h-208Zm208 185h-208v75h52v78h74v-78h82v-75ZM1170 1181a68 68 0 11-136 0 68 68 0 01 137 0Zm-684 397h349c16 0 28 12 28 28v205c0 16-12 30-28 30H486a29 29 0 01-28-30v-205c0-16 12-28 28-28Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M845 1708a79 79 0 11-157 0 79 79 0 01 157 0Zm-212 0a79 79 0 11-157 0 79 79 0 01 157 0Zm-510-228h220v970h-220Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M115 1463H350v30H115Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M82 751h246a30 30 0 01 29 29v664a30 30 0 01-30 29H83a29 29 0 01-28-30V780c0-16 12-29 28-29Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M39 1388h75v100h-40c-20 0-35-16-35-36v-64Zm0-160h75v88H39Zm0-160h75v88H39Zm0-160h75v89H39Zm36-171h39v100H39v-65c0-20 16-35 36-35Zm97 86H260v565h-88Zm1381 605a162 162 0 11-324 0 162 162 0 01 324 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1477 1428a87 87 0 11-173 0 87 87 0 01 173 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1456 1428a65 65 0 11-130 0 65 65 0 01 130 0ZM604 124h163v134H604Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M853 1402V167H517v1270c0 68 76 124 168 124 93 0 168-56 168-124ZM1981 236h65V290h-65Zm0 196h65v55h-65Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1643 178h345a30 30 0 01 29 30v308a30 30 0 01-30 29h-344a29 29 0 01-30-30V208a30 30 0 01 30-29Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1056 148h542c40 0 72 33 72 72v762c0 40-32 72-72 72h-542c-40 0-73-32-73-72V220c0-40 33-72 73-72Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1044 128h83v84h-83Zm483 0h83v84h-83Zm-322 0h83v84h-83Zm161 0h83v84h-83Zm246 111h74v84h-74Zm0 118h74v85h-74Zm0 486h74v84h-74ZM968 239h74v84h-74Zm0 118h74v85h-74Zm0 486h74v84h-74Zm613 152h105v18c0 32-26 57-58 57h-47v-75Zm-613 0h105v75h-47a58 58 0 01-58-58v-17Zm491 0h104v75H1460Zm-123 0H1440v75h-104Zm-122 0h104v75h-104Zm-124 0h105v75h-104Zm-2-575h477v286h-478Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1088 507h477v200h-478Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1088 704v-200c-2-44-105-40-105-40v281s104-2 105-41Zm582 70V440c0 52-41 49-41 49h-64v235h58s48-4 47 49Zm-372-381a85 85 0 01 62-158c80 30 98 21 100 24 6 5-18-7-52 86a84 84 0 01-110 48ZM1460 259l-208 88" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1470 270a38 38 0 11-77 0 38 38 0 01 76 0Zm-338 1703a101 101 0 11-202 0 101 101 0 01 202 0Zm-310 71a100 100 0 11-202 0 100 100 0 01 202 0ZM1918 382h40v41h-40Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1957 330h-38l-19 32 20 32h37l19-32-20-33Zm-171-2h-42l-20 37 20 36h42l20-36-20-37Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1734 386h62v67h-62Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1740 453h49c4 0 7 3 7 6 0 4-3 7-7 7H1740a7 7 0 11 0-14Zm125 2175 13 35a22 22 0 11-40 15l-13-34Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m1880 2640-58 22-23-63 57-21Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m1850 2580-45 17c-3 1-7 0-8-4-2-4 0-7 4-9l45-16c3-2 7 0 8 3 1 4 0 8-4 10Zm-64 150-21-57 54-20 10 4c7 0 13 0 20-3l17 46Zm-22-58 21 58-12 4-21-57Zm11 29-11 4m-751-1143h56v75h-56Zm56 38h-56m0 91h56v75h-56Zm0 38h56" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M864 1486H1030v378H864Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1030 1488c0 60-45 107-83 107-39 0-83-48-83-107 0-60 44-107 83-107 38 0 83 48 83 107Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M463 643h445v824H463Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M558 830h254v44H558Zm0 167h254v44H558Zm0 165h254v45H558Zm2 305v-100h66l31-62h57l30 61h67v101H560Zm-2-824h254v70H558Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M537 665h40v747h-40Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M577 665c0 12-10 22-20 22-11 0-20-10-20-22 0-11 9-21 20-21 10 0 20 10 20 21Zm0 750c0 29-10 52-20 52-11 0-20-23-20-52 0-30 9-53 20-53 10 0 20 24 20 53Zm217-751h40V1410h-40Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M834 665c0 11-9 21-20 21-10 0-20-10-20-21 0-12 10-22 20-22 11 0 20 10 20 22Zm0 749c0 30-9 53-20 53-10 0-20-24-20-53 0-30 10-53 20-53 11 0 20 24 20 53ZM555 830H810v44H555Zm0 167H810v45H555Zm0 166H810v44H555Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M555 643H810v70H555Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M533 665h40v747h-40Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M573 665c0 12-9 22-20 22-10 0-20-10-20-22 0-11 10-21 20-21 11 0 20 10 20 21Zm218-1h40v748h-40Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M831 665a20 20 0 01-20 21c-10 0-20-10-20-21 0-12 10-22 20-22 11 0 20 10 20 22ZM710 678a25 25 0 11-50 0 25 25 0 01 50 0ZM1811 2580c-28-70-36-132-46-216-5-42-11-90-20-147-19-109-65-260-180-355-130-107-331-115-494-120l1-25c166 6 373 14 508 126 122 100 170 257 188 370 10 58 16 106 20 148 10 82 19 142 46 210Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1332 1963c-10-10-10-25 0-34l60-60a110 110 0 01 156 0l60 60c10 10 10 25 0 34l-60 60a110 110 0 01-156 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1564 1946a94 94 0 11-188 0 94 94 0 01 188 0Zm-506 19-35-20 21-36 35 20Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m998 1990 32 20 33-19v-37l-32-20-33 20v37Zm249-376a4514 4514 0 01-177-2v-24h86a1720 1720 0 00 236-6 426 426 0 00 350-324 628 628 0 00 7-306l-10-61c-12-78-3-174 5-266 5-57 10-110 10-157h24c0 48-5 102-10 159-8 90-16 185-5 260l10 60c10 42 15 84 16 126a810 810 0 01-23 190 450 450 0 01-391 346c-43 4-85 6-128 5ZM515 236h-28c0-40-2-80-6-120-8-77-90-74-93-73-4 0-77-1-80 48-3 12 0 25 8 34 9 7 20 10 30 10h50v28h-48a70 70 0 01-52-18 67 67 0 01-17-56c5-60 73-75 109-75 38-2 113 16 121 100 5 40 7 81 6 122Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1745 1242a730 730 0 00 20-170c-1-40-7-80-16-120l-10-61c-12-78-3-174 5-266 5-57 10-110 10-157h24c0 48-5 102-10 159-8 90-16 185-5 260l10 60c10 42 15 84 16 126a730 730 0 01-10 130" fill="none" stroke="#000" stroke-width="4"/>
  <image xlink:href="data:image/png;base64, iVBORw0KGgoAAAANSUhEUgAAAAkAAAASCAYAAACJgPRIAAAAAXNSR0IArs4c6QAAAERlWElmTU0AKgAAAAgAAYdpAAQAAAABAAAAGgAAAAAAA6ABAAMAAAABAAEAAKACAAQAAAABAAAACaADAAQAAAABAAAAEgAAAAA4cj8yAAACH0lEQVQoFWNgwA0kgVKeIGlmNDUCQL6pjIxMso+PT9rHjx+lgPgTC1CQEYj1ra2tfTU1NT2AQF5XV1fizJkzd0+fPv0MKHcFiBlcAwICVt69e/fr8uXLj0VGRi4XFxefICcnt4iDg2MhUN4SpEiZiYlpKTc3d6+hoeGOkpKSC4cOHXpz7ty5ny4uLvOA8nogq8xiYmIS8vLyEoHsjwcOHLiyc+fO1ydPnvzw5csXCaBYHBAzGEhISJSqq6tPFRERWezu7n549uzZN4Dgm6+vbxNQXhikSEhRUXHB5s2bH967d+/X+vXrL6WkpGwGemIuUG4CSAETEIu+f//+y8KFCzcbGBhMB7rpLtDBYlOnTnUVFBQUAikCBcHN79+/f5WUlLTZtGmTFi8vL8PevXuvtra2ngVqPg1SBDKJAeg7BiUlpbtlZWUbPT09txw5cuRfWFiYIiMjoxNIHmQSw9+/f3/s2bNHsaCgQMXS0lIU6LNb69ate/X///+NcEVAk+SAjn2zevXqR0CFv4EBKW1vbw+yRR+kCGzdjx8/rvf29jJraWlZ7t+/P7KtrU37169fX4Hyx0CKwNYxMzPrdXd3cwCD4WRoaOjHx48fM/Pz83MD5cFhBE4FQLuZgD7j5eTkVMvKyjJqamqyBUbTc6AvLwIVXgBFCwgUA03R/f37tzKQvgaMmqP379+/BBQH4X8gBSCgBMSTgNgMiEFpCgUAAJRS0xOVqEBlAAAAAElFTkSuQmCC" width="90" height="180" x="180" y="2440"/>
  <path d="m244 2634 2-22h-43l2 22h40Zm13-164h-65v-21h65Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <image xlink:href="data:image/png;base64, iVBORw0KGgoAAAANSUhEUgAAAA8AAAARCAYAAAACCvahAAAAAXNSR0IArs4c6QAAAERlWElmTU0AKgAAAAgAAYdpAAQAAAABAAAAGgAAAAAAA6ABAAMAAAABAAEAAKACAAQAAAABAAAAD6ADAAQAAAABAAAAEQAAAADY5vLqAAACI0lEQVQ4EWNgQAV6QO4hIFZDFcbOY0IS5gsMDKycMmUKB1DMH4g1keTwMhm1tLQST5w48dXW1naJjo7OPgEBgT68OoCSMJsNs7Kysnfu3Hn/169fH6ZNm2bDy8v7HyjfD8S6+AwRcnR0bDp8+PBHcXHx5jVr1jxMSEg4Ym1tfaS3t/ckUGMFEAvjMkCShYVljaio6MqMjIw1a9eufaOiotJ/6tSpb2ZmZkuB3tjPz8/fgUszSFwciFNu3rz5wsDAYP7kyZMv1NTUXHZzc1sAMgRocBVQvhWIccZCJtAFu/z9/WcdOHDgs7S0dM+WLVveBAUFbQe64EB9ff1hoOYiIJYEYjCABRiIM/3Pnz+LvLy8/pWWlu6PiYnRf/bs2Y+LFy+enTRpktm+ffseqKurWzExMU2EaMUkZYFCE/T19ZsvXLjwQ0NDA6hv0qXa2tpLwACcA4yNn8LCwiDnVwOxADOa/k9A/isuLi6Nbdu2/Qdq/gl0iVVdXd3ytra28OXLl59mZWXlB6YDidu3b79F0wvnugGdV3z+/Pk3Li4uS4He2DN//vyn8vLyTUeOHPnq4eGxAKiSH64aC6MGGGgbzc3Nu4CG/AS6YkpXV9fZzs7Oa0C1eBMOyCxQGk9evXr18vDw8B1A588A2vodGGiZIEligDpQ0R4+Pr7CQ4cOvQTGwGogn4cYjXA1goKCLUlJSQfY2NiIzmlwzUBGJBCbIQuA2AAwocBF12XfDgAAAABJRU5ErkJggg==" width="150" height="170" x="290" y="3290"/>
  <path d="m284 3307 13 18 32-28-16-15-30 25Zm98 133 49-42 13 15-49 42ZM176 186h288V570H176Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M334 101h40v111h-40ZM407 540h40v741h-40Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M319 186h145V570H319Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M417 275a26 26 0 11-51 0 26 26 0 01 50 0Zm0 205a26 26 0 11-51 0 26 26 0 01 50 0Zm-140-179v-28l-5-12v-25h20v25l-7 12V300M66 334h89V430H66Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M143 326h32v112h-32Zm104-32h71v160h-70Zm289 228h-70V237h70Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M480 293h-15v-56H480Zm14 229h-29V348h29ZM172 823H260v565h-88Zm132-156h-50v-84h60l1 57v27h-11Zm11-86h-62V570h62Zm-31 1V570" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M315 670a30 30 0 11-62 0 30 30 0 01 62 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M284 682c-22 0-42-10-56-26a75 75 0 01-15-64 82 82 0 01 29-43c7-6 13-13 18-20 6-13 9-27 8-40v-35h25v35c0 17-4 35-12 50-6 10-14 20-23 28-10 7-17 18-20 30a50 50 0 00 10 43 46 46 0 00 36 18 12 12 0 01 0 24Zm1790-50h18v276h-18Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2092 648h17v244h-17Zm17-33h17V920H2110Zm-377 2939 118-136 19 16L1750 3570Zm38-4 82-95 65 57-81 94Zm785-755v628h-117v-628Zm0 628-9 22h-94l-14-22h117Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2575 3284v70h-158v-70Zm-41-2381v140h-166v-140Zm41 1735v200h-158v-200Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2585 1792v956h-177v-956Zm235-773v133h-543V1020ZM2618 3350v-53h-43v130s60-75 120 2c0 0-4-76-77-79ZM2750 1778V1990h-150v-212ZM2554 262H2770v690h-216Zm-200 923v93H2310v-93Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2380 1164v134h-30v-134Zm2 148h-67v13h40v21h-40v13h40v22h-40v13h40v21h-40v13h40v22h-40v13h40v22h-40v12h40v22h-40v13h40v22h-40v13h40v21h-40v13h67v-290Zm435 174v102h-31v-102Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2791 1436v165h-45v-165Zm-5-94V1240h31v102Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2746 1391v-164h45V1390Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2762 1153v643h-390v-643Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2744 1608v88h-70v-88Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2751 1591v33h-84v-33Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2372 1346v255h390v-355a98 98 0 01-99 100Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2573 1242v-1020H2750v1020a89 89 0 11-178 0Zm27 301c0 7-6 12-12 12h-112c-6 0-11-5-11-11v-1c0-6 5-11 10-11h113c6 0 11 5 11 10Zm0-55a10 10 0 01-11 12h-113a11 11 0 01 0-23h113c6 0 10 5 10 11Zm0-55c0 7-5 12-11 12h-113a11 11 0 01 0-23h113c6 0 10 5 10 11Zm320-458v310h-126v-310Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2920 1197v88h-126v-88Zm-170 291a67 67 0 11-135 0 67 67 0 01 134 0Zm-340 1160v100h-195v-100Zm280 13v73h-124v-73Zm-8-1263 18-30h-35l17 30Zm-256 245h186V1890h-186Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2426 1853h186v57h-186Zm-46-206h37v30H2380Zm241 0h37v30h-37ZM2468 1910H2570v80h-103Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2374 1600h290v50h-290Zm275 1732-14-20c144-97 197-267 216-394 12-85 14-170 6-256v-2c0-7-3-70-68-70h-1c-14 0-28 5-37 16-23 24-25 73-24 90l-24 2c0-3-5-72 30-109a73 73 0 01 55-23c48 0 70 25 82 47 7 14 10 30 12 46 8 87 6 175-7 261-28 190-106 332-226 412Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2408 1162h57v153h-57Z" fill="none" stroke="#fff" stroke-width="4"/>
  <path d="M2419 1172h36v133h-36ZM2740 2673v50h-50v-50Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2739 2682h10v30h-10Zm-73 616v48h-48v-48Zm0 8h11v31h-10Zm99-2306h14v226h-14ZM2430 856h34v17H2430Zm-90 211h-18l-10 16 10 16h18l10-16-10-16Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2540 868v50h-180v-50Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2320 1083V902c-2-20-1-40 0-60 6-36 30-73 72-73 50 0 65 56 66 85h-20c0-2-3-65-46-65-30 0-47 28-50 56a220 220 0 00-2 57v181c0 4-3 7-8 8-5 0-10-3-12-8Zm440-86h23v118H2760Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <g stroke="none">
    <path d="M1914 432v-6l2-2h43l2 2v6c0 1 0 2-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1916 435h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-3 0-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1914 444v-6l2-2h43l2 2v7l-2 1h-43s-2 0-2-2Z" fill="#fff"/>
    <path d="M1916 447h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-3 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1914 456V450l2-2h43l2 3v6l-2 2h-43l-2-3Z" fill="#fff"/>
    <path d="M1916 460h43c2 0 4-2 4-4V450c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v7h-42V450Z" fill="#000"/>
    <path d="M1914 469v-7l2-1h43s2 0 2 2v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1916 472h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 0-4 3v6c0 1 2 3 4 3Zm43-10v8h-43v-8Z" fill="#000"/>
    <path d="M1914 480v-6l2-1h43s2 0 2 2v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1916 484h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 1 2 3 4 3Zm42-10v7h-42v-7Z" fill="#000"/>
    <path d="M1914 493v-6c0-1 0-2 2-2h43l1 2v6l-1 2h-43c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="M1916 496h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1914 505V500c0-1 0-2 2-2h43l1 2v7l-1 1h-43c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="M1916 508h43c2 0 4 0 4-3V500c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6l4 2Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1914 518V510s0-2 2-2h42l2 3v6s0 2-2 2h-42l-2-2Z" fill="#fff"/>
    <path d="M1916 520h43c2 0 4 0 4-2V510c0-1-2-3-4-3h-43c-2 0-4 2-4 3v7c0 1 1 3 4 3Zm42-10v8h-42v-7Z" fill="#000"/>
    <path d="M1914 530v-7l2-1h42c1 0 2 0 2 2v6c0 1 0 2-2 2h-43l-1-2Z" fill="#fff"/>
    <path d="M1916 533h42c3 0 4-2 4-3v-6c0-2-1-3-3-3h-43c-3 0-4 0-4 3v6c0 1 1 3 4 3Zm42-10v7h-42v-7Z" fill="#000"/>
    <path d="M1914 542v-6l1-2h43l2 2v6c0 1 0 2-2 2h-43l-1-2Z" fill="#fff"/>
    <path d="M1916 545h42c3 0 4-1 4-3v-6c0-2-1-3-4-3h-42c-3 0-5 0-5 3v6c0 2 2 3 5 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1914 554v-6l1-2h43l2 2v7l-2 1h-43l-1-2Z" fill="#fff"/>
    <path d="M1915 557h43c2 0 4 0 4-3v-6c0-2-1-3-4-3h-42c-3 0-5 1-5 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1913 566V560l2-2h43l2 2v7l-2 2h-43l-2-3Z" fill="#fff"/>
    <path d="M1915 570h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 1 2 3 4 3Zm43-10v8h-42V560Z" fill="#000"/>
    <path d="M1913 579v-7l2-1h43s2 0 2 2v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1915 582h43c2 0 4-2 4-3v-6c0-2-2-3-4-4h-43c-2 0-4 2-4 3v7c0 1 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1913 590v-5l2-2h43s2 0 2 2v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1915 594h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1913 603v-6l2-2h43l2 2v7l-2 1h-43c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="M1915 606h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1913 615V610s0-2 2-2h43l1 2v7l-1 2h-43l-2-3Z" fill="#fff"/>
    <path d="M1915 618h43c2 0 4 0 4-3V610c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6l4 2Zm42-10v8h-41v-8Z" fill="#000"/>
    <path d="M1913 628V620l2-1h43l1 2v6c0 1 0 2-2 2h-42l-2-2Z" fill="#fff"/>
    <path d="M1915 630h43c2 0 4 0 4-2V620l-4-2h-43c-2 0-4 0-4 2v7c0 2 2 3 4 3Zm42-10v8h-42v-7Z" fill="#000"/>
    <path d="M1913 640v-6c0-1 0-2 2-2h43l1 2v6c0 1 0 2-2 2h-42l-2-2Z" fill="#fff"/>
    <path d="M1915 643h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-3 0-4 1-4 3v6c0 1 2 3 4 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1913 652v-6c0-1 0-2 2-2h42l2 2v7l-2 1h-42c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="M1915 655h42c3 0 4-1 4-3v-6c0-2-1-3-4-3h-42c-2 0-4 0-4 3v6c0 2 1 3 4 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1913 664v-6l1-2h43l2 2v7l-2 1h-43l-1-2Z" fill="#fff"/>
    <path d="M1914 667h43c3 0 4 0 4-3v-6c0-2-1-3-4-3h-43l-4 3v6c0 2 3 4 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1913 677V670l1-2h43l2 3v6s0 2-2 2h-43l-1-2Z" fill="#fff"/>
    <path d="M1914 680h43c2 0 4-1 4-3v-6c0-2-1-3-4-3h-43c-2 0-3 1-3 3v6c-1 1 1 3 3 3Zm43-10v8h-42V670Z" fill="#000"/>
    <path d="M1912 689v-7l2-1h43c1 0 2 0 2 2v6c0 1 0 2-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1914 692h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 0-4 3v6c0 1 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1912 700v-5l2-2h43c1 0 2 0 2 2v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1914 704h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1912 713v-6l2-2h43l2 2v7l-2 1h-43s-2 0-2-2Z" fill="#fff"/>
    <path d="M1914 716h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1912 725V720l2-2h43l2 2v7l-2 2h-43l-2-3Z" fill="#fff"/>
    <path d="M1914 729h43c2 0 4-2 4-4V720c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 1 2 3 4 3Zm42-10v7h-42v-8Z" fill="#000"/>
    <path d="M1912 738V730l2-1h43s2 0 2 2v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1914 740h43c2 0 4 0 4-2v-6c0-2-2-3-4-3h-43c-2 0-4 0-4 3v6c0 1 2 3 4 3Zm42-10v8h-42v-7Z" fill="#000"/>
    <path d="M1912 750v-6c0-1 0-2 2-2h43s2 0 2 2v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1914 753h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1912 762v-6c0-1 0-2 2-2h43l1 2v6c0 1 0 2-2 2h-42c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="M1914 765h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 1 3 4 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1912 774v-6s0-2 2-2h42l2 2v7s0 2-2 2h-42l-2-3Z" fill="#fff"/>
    <path d="M1914 778h42c3 0 5-2 5-3v-7c0-1-2-3-4-3h-43c-3 0-4 2-4 3v7c0 1 1 2 4 3Zm42-10v7h-42v-8Z" fill="#000"/>
    <path d="M1912 787V780s0-2 2-1h42c1 0 2 0 2 2v6s0 2-2 2h-42l-2-2Z" fill="#fff"/>
    <path d="M1914 790h42c3 0 5-1 5-3v-6c0-2-2-3-5-3h-42c-3 0-5 1-5 3v6c1 1 2 3 5 3Zm42-10v8h-42V780Z" fill="#000"/>
    <path d="M1912 799v-6c0-1 0-2 2-2h42c1 0 2 0 2 2v6c0 1 0 2-2 2h-43l-1-2Z" fill="#fff"/>
    <path d="M1914 802h42c3 0 4-2 4-3v-6c0-2-1-3-4-3h-42c-3 0-5 0-5 3v6c0 1 2 3 5 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1911 811v-6l2-2h43l2 2v6c0 1 0 2-2 2h-43s-2 0-2-2Z" fill="#fff"/>
    <path d="M1913 814h43c3 0 4-1 4-3v-6c0-2-1-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1911 823v-6l2-2h43l2 2v7s0 2-2 2h-43l-2-3Z" fill="#fff"/>
    <path d="M1913 826h43c2 0 4 0 4-2v-7c0-1-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1911 836v-7l2-2h43l2 3v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1913 839h43c2 0 4-2 4-3V830c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 1 2 3 4 3Zm43-10v7h-42V830Z" fill="#000"/>
    <path d="M1911 848V840l2-1h43s2 0 2 2v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1913 850h43c2 0 4 0 4-2v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v9h-42V840Z" fill="#000"/>
    <path d="M1911 860v-6l2-2h43l2 2v6l-2 2h-43c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="M1913 863h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1911 872v-6c0-1 0-2 2-2h43l2 2v7l-2 1h-43c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="M1913 875h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm42-10v8h-41v-8Z" fill="#000"/>
    <path d="m1910 885 1-7s0-2 2-2h43l1 3v6l-1 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1913 888h43c2 0 3-1 4-3v-7c0-1-2-3-4-3h-43c-2 0-4 1-4 3v7c0 1 2 3 4 3Zm42-10v7h-42v-7Z" fill="#000"/>
    <path d="M1910 897V890l3-1h43l1 2v6c0 1 0 2-2 2h-42l-2-2Z" fill="#fff"/>
    <path d="M1913 900h43c2 0 3-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 1 2 3 4 3Zm42-10v8h-42V890Z" fill="#000"/>
    <path d="M1910 909v-6l3-2h42c1 0 2 0 2 2v6c0 1 0 2-2 2h-42l-2-2Z" fill="#fff"/>
    <path d="M1913 912h43l4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 1 3 4 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1910 921v-6l3-2h42l2 2v7l-2 1h-42c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="M1913 924h42c3 0 4-1 4-3v-6c0-2-1-3-3-3h-43c-3 0-4 1-4 3v6c0 2 1 3 4 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1920 940v-7l1-2h43l2 2v7l-2 2H1920l-2-3Z" fill="#fff"/>
    <path d="M1913 936h42c3 0 4 0 4-2v-7c0-1-1-3-4-3h-42c-3 0-4 2-4 3v7c0 1 1 2 3 2Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="M1910 946V940l3-2h42c1 0 2 0 2 2v6c0 1 0 2-2 2h-43l-1-2Z" fill="#fff"/>
    <path d="M1912 949h43c3 0 4-2 4-3V940c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 1 2 3 4 3Zm43-10v7h-42V940Z" fill="#000"/>
    <path d="M1900 951v-6l3-2h43l1 2v6l-1 2h-43c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="M1912 961h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42V950Z" fill="#000"/>
    <path d="M1910 970v-6l2-2h43l2 2v6c0 1 0 2-2 2h-43l-1-2Z" fill="#fff"/>
    <path d="M1912 973h43c2 0 4-1 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1910 983v-7l2-2h43l2 3v6s0 2-2 2h-43l-1-2Z" fill="#fff"/>
    <path d="M1912 985h43c2 0 4 0 4-3v-6c0-1-2-3-4-3h-43c-2 0-4 1-4 3v6c0 2 2 3 4 3Zm43-10v8h-42v-8Z" fill="#000"/>
    <path d="M1910 995v-7l2-1h43c1 0 2 0 2 2v6l-2 2h-43l-2-2Z" fill="#fff"/>
    <path d="M1912 998h43c2 0 4-2 4-3v-7c0-1-2-3-4-3h-43c-2 0-4 1-4 3v7c0 1 2 3 4 3Zm43-10v7h-42v-7Z" fill="#000"/>
    <path d="M1955 1010h-43l-2-3v-6l2-2h43l2 2v6l-2 2Z" fill="#fff"/>
    <path d="M1912 1010h43c2 0 4 0 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 2-4 3v6c0 1 2 3 4 3Zm42-10v8h-41v-8Z" fill="#000"/>
    <path d="M1955 1021h-43l-2-2v-6c0-1 0-2 2-2h43s2 0 1 2v7l-1 1Z" fill="#fff"/>
    <path d="M1912 1022h43c2 0 4 0 4-2v-7c0-1-2-3-4-3h-43c-2 0-4 1-4 3v6c0 1 2 3 4 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="m1954 1034-42-1c-1 0-2 0-2-2v-6c0-1 0-2 2-2h42c2 0 2 2 2 3v6s0 2-2 2Z" fill="#fff"/>
    <path d="M1912 1034h42c3 0 5 0 5-2v-6c0-2-2-3-5-3l-42-1c-2 0-4 0-4 3v6c0 2 1 3 4 3Zm42-10v8h-42v-8Z" fill="#000"/>
    <path d="m1954 1046-43-1-1-2v-6a2 2 0 01 1-2l43 1a2 2 0 01 2 2v6c0 1 0 2-2 2Z" fill="#fff"/>
    <path d="m1911 1046 43 1c2 0 4 0 4-3v-6c0-2-2-3-4-3h-43c-2 0-4 0-4 2v6c0 2 2 3 4 3Zm43-9v8l-42-1v-8Z" fill="#000"/>
    <path d="M1954 1058H1910l-2-3v-6l2-2 43 1a2 2 0 01 2 2v7l-2 1Z" fill="#fff"/>
    <path d="m1911 1058 43 2c2 0 4-1 4-3v-7c0-2-2-3-4-3H1910c-2 0-4 0-4 2v6c0 2 2 3 4 3Zm43-8v7h-42v-9Z" fill="#000"/>
    <path d="M1954 1070H1910a2 2 0 01-2-3v-6s0-2 2-2l43 2 1 2v6a2 2 0 01-1 2Z" fill="#fff"/>
    <path d="m1910 1070 43 2c3 0 5-2 5-3v-7c0-1-2-3-4-3l-43-1c-2 0-4 1-4 3v6c0 2 1 3 4 3Zm43-9v8l-42-1v-8Z" fill="#000"/>
    <path d="m1953 1083-43-2a2 2 0 01-1-2v-6l1-2 43 2a2 2 0 01 2 2v6c0 1 0 2-2 2Z" fill="#fff"/>
    <path d="m1910 1083 43 1c2 0 4 0 4-3v-6c0-1-2-3-4-3l-43-2c-2 0-4 2-4 3v6c0 2 2 3 4 3Zm43-9v8l-42-2v-7Z" fill="#000"/>
    <path d="m1953 1095-43-2a2 2 0 01-2-2v-6a2 2 0 01 2-1l43 1a2 2 0 01 1 2v7s0 2-2 1Z" fill="#fff"/>
    <path d="m1910 1095 43 2c2 0 4-2 4-3v-7c0-1-2-3-4-3l-43-2c-2 0-4 2-4 3v6c0 2 2 4 4 4Zm43-8v7l-42-2v-7Z" fill="#000"/>
    <path d="m1952 1108-43-3-2-1v-7a2 2 0 01 2-1l43 2c1 0 2 0 2 2v6l-2 2Z" fill="#fff"/>
    <path d="m1909 1107 43 2c2 0 4-1 4-3v-6c0-2-2-3-4-3l-43-2c-2 0-4 0-4 2v6c0 2 2 3 4 4Zm43-8v8l-43-3 1-7Z" fill="#000"/>
    <path d="m1951 1120-43-3a2 2 0 01-1-2v-6a2 2 0 01 2-2l42 3a2 2 0 01 2 2v6c0 1 0 2-2 2Z" fill="#fff"/>
    <path d="m1908 1119 43 2c2 0 4 0 4-2v-7c0-1-1-3-4-3l-42-2c-3 0-5 0-5 2v6c0 2 2 4 4 4Zm43-8v8l-42-3v-7Z" fill="#000"/>
    <path d="m1950 1133-43-4a2 2 0 01-1-2v-6l2-1 42 3c1 0 2 0 2 2v6a2 2 0 01-2 2Z" fill="#fff"/>
    <path d="m1907 1130 43 4c2 0 4 0 4-3v-6c0-2-1-3-3-4l-43-2c-2 0-4 0-4 2v6c0 2 1 3 3 4Zm43-6v7l-42-3v-7Z" fill="#000"/>
    <path d="m1950 1145-44-4-1-1v-7l2-1 43 3a2 2 0 01 2 2v6a2 2 0 01-3 2Z" fill="#fff"/>
    <path d="m1906 1143 43 3c2 0 4 0 4-2v-7c0-1 0-3-3-3l-43-3c-2 0-4 0-4 2v6c0 2 1 3 3 4Zm43-7v8l-42-4v-7Z" fill="#000"/>
    <path d="m1948 1158-42-5c-2 0-2 0-2-2v-6a2 2 0 01 2-1l43 3 2 2-1 7a2 2 0 01-2 2Z" fill="#fff"/>
    <path d="m1905 1155 43 4c2 0 4-1 4-3v-6c0-2 0-4-3-4l-43-3c-2 0-4 0-4 2v6c0 2 1 3 3 4Zm43-6v7l-42-4v-7Z" fill="#000"/>
    <path d="m1947 1170-43-5a2 2 0 01-2-2l1-6a2 2 0 01 2-1l43 4a2 2 0 01 1 2v6a2 2 0 01-2 2Z" fill="#fff"/>
    <path d="m1904 1166 43 5c2 0 3 0 4-2v-7c0-2-1-3-3-3l-43-4c-2 0-4 0-4 2v6c-1 2 0 3 3 3Zm43-5v8l-42-5v-7Z" fill="#000"/>
    <path d="m1945 1182-42-5c-1 0-2 0-2-2v-6a2 2 0 01 3-2l42 5 2 2v7a2 2 0 01-3 1Z" fill="#fff"/>
    <path d="m1903 1178 42 6c2 0 4-1 5-3v-6c0-2-1-4-3-4l-43-5c-2 0-4 1-4 3l-1 6c0 1 2 3 4 3Zm43-5-1 8-42-5 1-7Z" fill="#000"/>
    <path d="m1944 1195-43-6a2 2 0 01-1-2v-6a2 2 0 01 3-2l42 6a2 2 0 01 2 2l-1 6-2 2Z" fill="#fff"/>
    <path d="m1901 1190 43 6c1 0 3 0 4-3v-6c0-2 0-3-3-4l-42-5c-2 0-4 0-5 3v6c0 2 0 3 3 3Zm43-4v7l-42-5v-8Z" fill="#000"/>
    <path d="m1942 1207-42-6a2 2 0 01-2-2v-6a2 2 0 01 3-2l42 6a2 2 0 01 2 2l-1 6a2 2 0 01-2 2Z" fill="#fff"/>
    <path d="m1900 1202 42 6c2 0 4 0 4-2l1-6c0-2-1-4-3-4l-43-6c-2 0-4 0-4 3l-1 6c0 1 1 3 3 3Zm43-4-1 8-42-6 1-7Z" fill="#000"/>
    <path d="m1940 1220-42-7a2 2 0 01-2-2l1-6a2 2 0 01 2-1l42 6a2 2 0 01 2 2v7a2 2 0 01-3 1Z" fill="#fff"/>
    <path d="m1898 1214 42 7c2 0 4-1 4-3l1-6c0-2-1-3-3-4l-43-6c-2 0-4 0-4 2v6c-1 2 0 3 3 4Zm43-3-1 7-41-6v-8Z" fill="#000"/>
    <path d="m1938 1232-42-8c-1 0-2 0-2-2l1-6a2 2 0 01 3-1l42 7a2 2 0 01 1 2l-1 6a2 2 0 01-2 2Z" fill="#fff"/>
    <path d="m1896 1225 42 8c2 0 4 0 4-2l1-6c0-2 0-4-3-4l-42-7c-3 0-5 0-5 2l-1 6c0 2 1 3 4 3Zm43-2-1 8-42-8 2-7Z" fill="#000"/>
    <path d="m1936 1244-42-8-2-2 1-6s1-2 2-1l43 7s1 1 0 2v7l-2 1Z" fill="#fff"/>
    <path d="m1893 1237 42 8c2 1 4 0 5-2l1-6c0-2-1-4-3-4l-42-8c-2 0-4 0-5 3l-1 6c0 1 1 3 3 3Zm44-2-2 8-40-8v-7Z" fill="#000"/>
    <path d="m1933 1257-42-10-1-1 1-6a2 2 0 01 2-2l42 9c1 0 2 0 2 2l-2 6c0 1 0 2-2 2Z" fill="#fff"/>
    <path d="m1891 1249 42 9c2 0 4 0 4-2l2-7c0-1-1-3-4-4l-42-8c-2 0-4 0-4 2l-1 6c0 2 0 3 3 4Zm44-1-2 7-41-8 1-7Z" fill="#000"/>
    <path d="m1930 1269-42-10-1-2 1-6c0-1 1-2 2-1l42 9a2 2 0 01 2 2l-2 6a2 2 0 01-2 2Z" fill="#fff"/>
    <path d="m1888 1260 42 10c2 0 4 0 5-2l1-6c0-2-1-4-3-4l-42-10c-2 0-4 0-5 3l-1 6c0 1 1 3 3 3Zm44 0-2 8-40-10v-7Z" fill="#000"/>
    <path d="m1928 1281-42-10a2 2 0 01-1-2l1-6a2 2 0 01 2-1l42 10a2 2 0 01 1 2l-1 6c0 1-1 2-3 1Z" fill="#fff"/>
    <path d="m1886 1272 41 10c2 0 4 0 5-2l1-6c0-2 0-4-3-4l-42-10c-2 0-4 0-4 2l-2 5c0 3 2 4 4 5Zm43 0-2 8-40-10 1-8Z" fill="#000"/>
    <path d="m1924 1293-41-10a2 2 0 01-1-3l1-6a2 2 0 01 2-1l42 10a2 2 0 01 1 2l-2 6a2 2 0 01-2 2Z" fill="#fff"/>
    <path d="m1883 1283 40 12a4 4 0 00 6-2l1-7c0-1 0-3-3-4l-41-10c-2 0-5 0-5 2l-2 6c0 1 1 3 4 3Zm43 2-2 7-40-10 2-8Z" fill="#000"/>
    <path d="m1920 1306-40-12-1-3 1-5a2 2 0 01 3-2l40 12c2 0 2 1 2 2l-2 6a2 2 0 01-2 2Z" fill="#fff"/>
    <path d="m1880 1295 41 12c2 0 4 0 5-2l2-6c0-2-1-4-4-4l-40-12c-3 0-5 0-6 2l-1 6c-1 1 0 3 3 4Zm43 2-2 7-40-11 2-7Z" fill="#000"/>
    <path d="m1917 1318-40-13c-2 0-2-1-2-2l2-6 2-1 41 12a2 2 0 01 1 2l-2 6c0 1 0 2-2 2Z" fill="#fff"/>
    <path d="m1876 1306 40 13c3 0 5 0 6-2l2-6c0-2-1-4-3-4l-42-12-4 1-2 6c0 2 0 3 3 4Zm44 3-3 7-40-12 2-7Z" fill="#000"/>
    <path d="m1913 1330-40-14c-1 0-2 0-2-2l2-5a2 2 0 01 3-2l40 13 2 3-2 6a2 2 0 01-3 1Z" fill="#fff"/>
    <path d="m1872 1317 41 14a4 4 0 00 5-2l2-6c0-2-1-4-3-4l-40-13c-3 0-5 0-6 2l-2 5c0 2 1 4 3 4Zm44 4-3 7-40-13 3-7Z" fill="#000"/>
    <path d="m1910 1342-40-15a2 2 0 01-2-2l2-6h2l40 13a2 2 0 01 2 2l-2 7a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1869 1329 40 14c1 0 4 0 5-2l2-6c0-2-1-4-3-4l-40-14c-3 0-5 0-6 2l-1 5c0 2 0 4 3 5Zm43 4-3 7-40-14 3-6Z" fill="#000"/>
    <path d="m1905 1354-40-16c-1 0-2 0-1-2l2-6a2 2 0 01 2 0l40 14a2 2 0 01 1 2l-2 6-2 2Z" fill="#fff"/>
    <path d="m1864 1340 40 15c2 1 4 0 5-1l3-6c0-2-1-4-3-5l-40-14-5 1-2 6s0 3 2 4Zm44 5-3 8-40-15 3-7Z" fill="#000"/>
    <path d="m1900 1365-40-16-1-2 2-6a2 2 0 01 2 0l40 15c1 0 2 1 1 2l-2 6c0 1-1 2-2 1Z" fill="#fff"/>
    <path d="m1860 1350 40 17a4 4 0 00 5-2l2-6c0-2 0-4-3-4l-40-15c-2-1-4 0-4 0l-3 6c0 2 1 4 3 4Zm43 7-3 7-39-15 3-7Z" fill="#000"/>
    <path d="m1895 1377-39-17-1-2 3-6a2 2 0 01 2 0l40 16a2 2 0 01 0 2l-2 6a2 2 0 01-3 1Z" fill="#fff"/>
    <path d="m1856 1361 39 17h5l3-7c0-2-1-4-3-4l-40-17c-2 0-4 0-5 2l-2 5c0 2 0 3 3 4Zm43 8-4 7-38-17 3-6Z" fill="#000"/>
    <path d="m1890 1389-39-18a2 2 0 01 0-2l2-6s1-1 2 0l40 16v3l-3 6a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1850 1372 40 18c2 1 4 0 5-1l3-6c0-2 0-4-3-5l-40-17-4 1-2 6c-1 1 0 3 2 4Zm44 8-4 7-38-17 3-7Z" fill="#000"/>
    <path d="m1885 1400-39-19v-2l2-5c0-1 1-2 2-1l40 18v2l-3 6-2 1Z" fill="#fff"/>
    <path d="m1846 1383 38 18c2 2 4 1 5 0l3-7c0-1 0-3-2-4l-39-18-5 1-2 6c-1 1 0 3 2 4Zm42 9-3 7-38-18 3-7Z" fill="#000"/>
    <path d="m1880 1412-39-20-1-2 3-5 2-1 39 19v2l-3 6s-1 1-2 0Z" fill="#fff"/>
    <path d="m1840 1393 39 20a4 4 0 00 5 0l2-7a4 4 0 00-2-4l-38-20c-2 0-4 0-5 2l-3 5c0 1 0 3 3 4Zm43 10-4 7-37-20 3-6Z" fill="#000"/>
    <path d="m1873 1423-37-20a2 2 0 01-1-3c0-2 2-4 3-5l2-1 38 20a2 2 0 01 0 2 108 108 0 01-4 7h-1Z" fill="#fff"/>
    <path d="m1835 1403 37 21h5l4-6c0-2 0-4-2-5l-38-20h-5l-3 6c-1 1 0 3 2 4Zm42 12-4 7-37-20 4-7Z" fill="#000"/>
    <path d="m1867 1434-37-22a2 2 0 01 0-2l2-5a2 2 0 01 3 0l37 20a2 2 0 01 0 3l-3 6a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1830 1413 36 22a4 4 0 00 5 0l3-6c1-2 0-4-2-5l-37-20a4 4 0 00-5 0l-3 4c0 2 0 4 2 5Zm40 13-3 7-36-22 3-6Z" fill="#000"/>
    <path d="m1860 1445-36-23a2 2 0 01 0-2l3-5a2 2 0 01 2 0l37 21v3l-3 6a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1823 1423 36 23c1 2 4 2 5 0l4-5a4 4 0 00-2-5l-37-22c-2-2-4-1-5 0l-3 5c-1 1 0 3 2 4Zm41 14-4 7-35-23 4-6Z" fill="#000"/>
    <path d="m1853 1456-36-24a2 2 0 01 0-3l4-5h2l36 23a2 2 0 01 0 3 106 106 0 01-5 6h-2Z" fill="#fff"/>
    <path d="m1817 1433 35 24h5l4-6c0-1 0-4-2-4l-36-24a4 4 0 00-5 0c0 2-2 3-3 5-1 2 0 3 2 4Zm40 15-4 7-35-24 4-6Z" fill="#000"/>
    <path d="m1845 1466-34-25c-1 0-1-2 0-2l3-5h2l35 24a2 2 0 01 0 3l-4 5h-2Z" fill="#fff"/>
    <path d="m1810 1442 34 26a3 3 0 00 5 0l4-6a4 4 0 00-1-5l-35-24a4 4 0 00-5 0l-4 4c0 2 0 4 2 5Zm40 17-5 6-33-25 4-6Z" fill="#000"/>
    <path d="m1837 1477-33-27c-1 0-1-2 0-2l3-5h3l34 26a2 2 0 01 0 2l-4 5a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1803 1451 33 27h5l5-6c0-1 0-3-2-4l-34-26a4 4 0 00-5 0l-4 5c-1 0 0 3 2 4Zm40 18-5 6-33-25 4-6Z" fill="#000"/>
    <path d="m1829 1487-33-28a2 2 0 01 0-2l4-5a2 2 0 01 2 0l34 27a2 2 0 01 0 2l-4 6h-3Z" fill="#fff"/>
    <path d="m1796 1460 32 28h5l4-6a4 4 0 00 0-4l-34-27c-2-1-4-2-5 0l-4 4c0 1 0 4 2 5Zm38 20-5 5-32-26 5-6Z" fill="#000"/>
    <path d="m1820 1496-31-28v-3l4-4a2 2 0 01 2 0l33 28v2l-5 5a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1788 1469 32 28a4 4 0 00 5 0l4-5a4 4 0 00 0-5l-33-27a4 4 0 00-5 0l-4 4v5Zm38 20-5 6-31-28 4-6Z" fill="#000"/>
    <path d="m1812 1506-31-30v-3l4-4a2 2 0 01 2 0l32 29a2 2 0 01 0 2l-4 5a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1780 1477 30 30c2 2 5 2 6 0l5-5a4 4 0 00-1-5l-32-28a4 4 0 00-5-1l-4 4c-1 1 0 4 1 5Zm38 22-6 6-30-30 5-4Z" fill="#000"/>
    <path d="m1803 1515-30-30a2 2 0 01 0-2l4-5a2 2 0 01 2 0l31 30a2 2 0 01 0 3l-4 5a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1772 1485 30 31c1 2 3 2 5 0l5-4-1-5-31-30c-2-2-4-2-5 0a80 80 0 01-4 3v5Zm37 23-6 6-30-30 5-5Z" fill="#000"/>
    <path d="m1793 1524-29-31a2 2 0 01 0-3l5-4h2l30 30a2 2 0 01 0 3l-5 5a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1764 1493 28 32c2 2 4 2 6 0l4-4a4 4 0 00 0-5l-30-30a4 4 0 00-5-2l-4 5c-1 1-1 3 0 4Zm36 24-6 6-29-30 6-6Z" fill="#000"/>
    <path d="m1784 1533-28-33v-3l4-3h3l29 31v3l-5 4a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1755 1501 28 32c1 2 3 2 5 1l5-4v-5l-30-32a4 4 0 00-4 0l-4 3c-2 1-2 3 0 5Zm36 25-7 5-27-31 5-5Z" fill="#000"/>
    <path d="m1774 1540-27-32a2 2 0 01 0-3l4-3h3l28 32a2 2 0 01 0 3l-5 4h-3Z" fill="#fff"/>
    <path d="m1746 1509 27 33c1 1 3 2 5 0 2 0 3-2 5-3 2-2 2-4 0-6l-28-32c-1-2-4-2-5-1l-4 4c-2 0-2 3 0 5Zm35 26-6 5-27-32 5-5Z" fill="#000"/>
    <path d="m1764 1549-26-34v-2l4-4h3l27 33a2 2 0 01 0 3l-5 4h-3Z" fill="#fff"/>
    <path d="m1737 1516 26 34c1 1 3 2 5 1l5-4a4 4 0 00 0-5l-27-33c-1-2-4-3-5-2l-4 4c-2 1-1 3 0 5Zm34 27-6 5-26-33 5-5Z" fill="#000"/>
    <path d="m1754 1557-26-35v-2l5-4h3l26 34a2 2 0 01 0 3l-5 4a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1728 1523 25 34c0 2 3 3 5 2l5-4c2-2 2-4 0-5l-26-34c-1-2-3-2-5-1l-5 3v5Zm33 28-6 5-25-34 6-4Z" fill="#000"/>
    <path d="m1744 1564-25-35v-2l5-4h2l26 35v2l-6 4h-2Z" fill="#fff"/>
    <path d="m1718 1530 25 35c0 2 3 2 4 1l6-3v-6l-26-34c0-2-3-2-4-2l-5 4c-2 1-1 3 0 5Zm33 29-7 4-24-34 6-4Z" fill="#000"/>
    <path d="m1733 1571-24-35a2 2 0 01 0-3l5-3a2 2 0 01 3 0l24 36v2l-6 4h-2Z" fill="#fff"/>
    <path d="m1708 1536 24 36a4 4 0 00 5 2l5-4c2-1 2-4 0-5l-24-35c-1-2-3-3-5-2l-5 4v4Zm32 30-6 4-23-34 5-5Z" fill="#000"/>
    <path d="m1723 1578-24-36 1-2 5-3h3l23 36a2 2 0 01 0 3l-6 3a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1699 1543 22 36c1 2 3 3 5 2l6-4v-5l-24-36c0-1-3-2-5-1l-5 3v5Zm31 30-7 4-22-35 6-4Z" fill="#000"/>
    <path d="M1712 1585 1690 1550v-3l6-3a2 2 0 01 2 0l23 37v2l-6 4a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1689 1550 22 36c0 2 3 2 5 2l5-4a4 4 0 00 0-5l-22-36c-1-2-3-3-5-2l-5 3c-2 1-2 3 0 5Zm30 30-6 4-22-36 6-4Z" fill="#000"/>
    <path d="m1700 1592-20-37v-3l5-3h3l22 37a2 2 0 01-1 3l-6 3h-2Z" fill="#fff"/>
    <path d="m1678 1555 22 37c0 2 3 3 4 2l6-3a4 4 0 00 0-5l-21-37c-2-2-4-3-5-2l-5 3c-2 1-2 3 0 5Zm30 32-7 4-20-37 6-3Z" fill="#000"/>
    <path d="m1690 1598-21-38v-2l6-3h2l22 37a2 2 0 01 0 3l-7 3h-2Z" fill="#fff"/>
    <path d="m1668 1561 20 37 6 3 5-4c2 0 2-3 0-5l-20-37c-2-2-4-3-5-2l-5 3c-2 1-2 3 0 5Zm30 32-7 4-21-37 6-4Z" fill="#000"/>
    <path d="m1679 1604-20-38a2 2 0 01 0-2l6-3a2 2 0 01 2 0l21 38a2 2 0 01 0 2l-6 4a2 2 0 01-3-1Z" fill="#fff"/>
    <path d="m1658 1567 20 37a4 4 0 00 4 3l6-3c1-2 2-4 0-6l-20-37c0-2-3-3-5-2l-5 3c-1 1-2 3 0 5Zm29 32-7 4-20-37 6-4Z" fill="#000"/>
    <path d="m1668 1610-20-38v-3l6-2h2l20 38v3l-6 3-2-1Z" fill="#fff"/>
    <path d="m1647 1572 20 38a4 4 0 00 4 3l6-4c1 0 2-3 0-5l-20-37c0-2-2-3-4-3l-5 3c-2 0-2 3 0 5Zm29 33-7 4-20-38 7-3Z" fill="#000"/>
    <path d="m1656 1615-18-38v-3l6-2a2 2 0 01 2 0l20 39-1 2-6 3a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1637 1577 18 39c0 2 3 3 5 2l6-3c1 0 2-3 0-5l-19-38c0-2-3-3-4-2l-6 3c-1 0-1 2 0 4Zm27 34-7 3-18-37 6-3Z" fill="#000"/>
    <path d="m1645 1620-18-38a2 2 0 01 1-3l5-2a2 2 0 01 3 0l18 40a2 2 0 01 0 2l-7 3-2-1Z" fill="#fff"/>
    <path d="m1626 1582 18 40 4 2 6-3c2-1 2-3 1-5l-18-39c-1-2-3-3-5-2l-5 3c-1 0-2 2 0 4Zm27 35-7 3-18-38 7-3Z" fill="#000"/>
    <path d="m1633 1626-17-40 1-2 6-2a2 2 0 01 2 0l18 40s0 2-2 2l-6 3h-2Z" fill="#fff"/>
    <path d="m1615 1587 17 40a4 4 0 00 4 2l6-2c2-1 3-3 2-5l-18-40-4-2-6 3c-1 0-2 2 0 4Zm26 35-7 3-16-39 6-2Z" fill="#000"/>
    <path d="m1621 1630-15-40v-2l6-2 2 1 17 40a2 2 0 01-1 3l-6 2a2 2 0 01-3-1Z" fill="#fff"/>
    <path d="m1604 1591 16 40a4 4 0 00 4 3l6-3c2 0 3-3 2-5l-17-40c0-2-2-3-4-2l-5 2a4 4 0 00-2 5Zm25 36-7 3-15-40 7-2Z" fill="#000"/>
    <path d="m1610 1635-15-40v-2l6-2h3l15 40a2 2 0 01-1 3l-7 3s-2 0-2-2Z" fill="#fff"/>
    <path d="m1594 1595 14 40c0 2 2 3 4 3l6-2c2-1 3-3 2-5l-16-40c0-2-2-3-4-3l-5 2c-2 1-2 3-1 5Zm24 37-8 2-14-40 7-2Z" fill="#000"/>
    <path d="m1597 1640-13-41a2 2 0 01 0-3l6-2c1 0 2 0 3 2l14 40a2 2 0 01-1 2l-7 2h-2Z" fill="#fff"/>
    <path d="m1582 1600 14 40a4 4 0 00 4 3l6-2a4 4 0 00 2-5l-14-40c-1-2-3-4-5-3l-5 2c-2 0-2 2-2 4Zm24 36-8 3-13-40 7-3Z" fill="#000"/>
    <path d="m1585 1643-13-40c0-2 0-3 2-3l5-2 2 1 14 41a2 2 0 01-1 2l-7 2h-2Z" fill="#fff"/>
    <path d="m1571 1603 13 40c0 3 2 4 4 4l6-2c2-1 3-3 2-5l-14-40c0-3-2-4-4-4l-5 2c-2 0-3 3-2 5Zm23 37-8 3-12-40 7-3Z" fill="#000"/>
    <path d="m1573 1647-12-41 1-2 6-2 2 1 13 40a2 2 0 01-2 3l-6 2a2 2 0 01-2-1Z" fill="#fff"/>
    <path d="m1560 1606 12 41c0 2 2 4 4 3l6-2a4 4 0 00 2-4l-13-41c0-2-2-4-4-3l-5 1c-2 0-3 3-2 5Zm22 38-8 2-12-40 7-2Z" fill="#000"/>
    <path d="m1560 1650-10-41 1-2 6-2 2 1 12 42a2 2 0 01-1 2l-6 1a2 2 0 01-3-1Z" fill="#fff"/>
    <path d="m1549 1610 10 40c0 3 3 4 5 4l6-2c2 0 2-3 2-5l-12-40c0-3-2-4-4-4l-6 2c-1 0-2 2-1 4Zm20 38-7 1-11-40 7-2Z" fill="#000"/>
    <path d="m1549 1654-10-42a2 2 0 01 0-2l6-2a2 2 0 01 2 2l12 40c0 2 0 3-2 3l-6 2-2-1Z" fill="#fff"/>
    <path d="m1537 1612 10 42c1 2 3 3 4 3l7-2 2-4-11-42c-1-2-3-3-4-3l-6 2c-2 0-3 2-2 4Zm20 39-8 2-10-40 8-3Z" fill="#000"/>
    <path d="m1536 1656-10-41a2 2 0 01 2-2l6-2a2 2 0 01 2 2l10 40a2 2 0 01-2 3l-6 2-2-2Z" fill="#fff"/>
    <path d="m1525 1615 10 42c0 2 3 3 4 3l7-1a4 4 0 00 2-5l-11-42c0-2-2-3-4-3l-6 2-2 4Zm20 39-7 2-10-41 7-2Z" fill="#000"/>
    <path d="m1524 1660-10-43 2-2 6-1 2 1 10 42a2 2 0 01-2 2l-6 2s-2 0-2-2Z" fill="#fff"/>
    <path d="m1514 1618 10 42c0 2 1 3 3 3l6-2c2 0 3-2 2-4l-10-42c0-2-2-4-4-3l-5 1c-2 0-3 3-2 5Zm19 39-8 2-9-41 7-2Z" fill="#000"/>
    <path d="m1512 1662-9-42 1-2 7-1 2 1 9 42a2 2 0 01-1 2l-7 1a2 2 0 01-2-1Z" fill="#fff"/>
    <path d="m1502 1620 9 42c0 2 2 4 4 3l6-1c2 0 3-3 2-5l-10-42c0-2-2-3-3-3l-6 1c-2 1-3 3-2 5Zm19 40h-8l-9-40 8-2Z" fill="#000"/>
    <path d="m1500 1664-9-41a2 2 0 01 2-2l6-2a2 2 0 01 2 2l9 42a2 2 0 01-2 2h-6a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1490 1623 9 42c0 2 2 3 3 3l7-1c2 0 2-3 2-5l-10-42c0-2-1-3-3-3l-6 1-2 5Zm19 40h-8l-9-40 8-2Z" fill="#000"/>
    <path d="m1488 1667-9-42a2 2 0 01 2-2l6-1 2 1 9 42c0 1 0 2-2 2l-6 1a2 2 0 01-2-1Z" fill="#fff"/>
    <path d="m1478 1625 9 42c0 2 2 4 3 4l7-2 2-4-9-42c0-2-2-4-4-4l-6 2-2 4Zm18 40-7 1-8-40 7-2Z" fill="#000"/>
    <path d="m1476 1670-8-43a2 2 0 01 0-2l7-1a2 2 0 01 2 2l9 42-2 2h-6a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1466 1628 8 42c1 2 3 3 4 3l6-1c2 0 3-3 3-5l-9-42c0-2-2-4-3-3l-6 1c-2 0-3 2-3 5Zm18 40h-7l-8-40 7-2Z" fill="#000"/>
    <path d="m1464 1672-8-42 1-3h6a2 2 0 01 2 0l9 43-2 2-6 1a2 2 0 01-2-1Z" fill="#fff"/>
    <path d="m1454 1630 8 42c0 2 3 4 4 4l6-2a4 4 0 00 2-4l-8-42c0-2-2-4-3-4l-6 1c-2 0-3 3-3 5Zm18 40-7 1-8-40 7-2Z" fill="#000"/>
    <path d="m1445 1630 6-1 2 1 8 42-1 2-6 1-2-1-8-42 1-2Z" fill="#fff"/>
    <path d="m1443 1632 7 42c0 2 3 4 4 4l6-2c2 0 3-2 3-4l-9-42c0-2-2-4-3-4l-6 2c-2 0-3 2-3 4Zm17 40-7 2-8-42h7Z" fill="#000"/>
    <path d="m1433 1632 6-1 2 1 8 42-1 3h-8l-8-43 1-2Z" fill="#fff"/>
    <path d="m1430 1634 8 42c0 3 2 4 4 4l6-1 2-5-8-42c0-2-2-4-4-3h-6c-2 2-3 4-2 5Zm18 40-8 2-8-41 8-2Z" fill="#000"/>
    <path d="m1428 1679-8-43a2 2 0 01 1-2h8l8 43a2 2 0 01-1 2l-6 1a2 2 0 01-2-1Z" fill="#fff"/>
    <path d="m1418 1637 8 42c0 2 3 3 4 3h6c2 0 3-3 2-6l-8-42c0-2-2-3-3-3l-6 1a4 4 0 00-3 5Zm18 40-7 1-8-41 7-1Z" fill="#000"/>
    <path d="m1416 1680-9-41c0-1 0-2 2-2l6-1h2l8 43-1 2-6 1a2 2 0 01-2-1Z" fill="#fff"/>
    <path d="m1406 1640 9 41c0 2 2 4 3 3h6a4 4 0 00 2-5l-8-42c0-3-2-4-3-4l-7 1-2 5Zm18 40h-7l-8-40 7-2Z" fill="#000"/>
    <path d="m1404 1683-9-42a2 2 0 01 2-2l6-1 2 1 9 42a2 2 0 01-2 2l-6 2s-2 0-2-2Z" fill="#fff"/>
    <path d="m1394 1642 9 42c0 2 2 3 3 3l7-1 2-5-9-42c0-2-2-3-4-3h-6c-2 2-3 4-2 6Zm18 40-7 1-9-40 8-2Z" fill="#000"/>
    <path d="m1392 1686-9-42a2 2 0 01 1-2l7-1a2 2 0 01 2 0l9 43a2 2 0 01-2 2l-6 1-2-1Z" fill="#fff"/>
    <path d="m1382 1644 10 42c0 2 1 4 3 3h6l2-5-10-42c0-3-1-4-3-4l-6 2c-2 0-3 2-2 4Zm19 40-8 1-9-40 8-3Z" fill="#000"/>
    <path d="m1380 1689-10-42 2-2 6-2c1 0 2 0 3 2l10 41-2 3h-6a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1370 1647 10 42c0 2 2 3 4 3l6-1 2-5-10-42c0-2-2-3-4-3l-6 2c-2 0-3 2-2 4Zm20 40h-8l-10-40 8-2Z" fill="#000"/>
    <path d="m1370 1691-11-41a2 2 0 01 0-2l7-2a2 2 0 01 2 2l10 41a2 2 0 01 0 2l-7 2s-2 0-2-2Z" fill="#fff"/>
    <path d="m1357 1650 11 41c0 3 2 4 4 4l6-2 2-4-10-42c0-2-3-4-4-3l-7 1-2 5Zm20 39-7 1-10-40 8-2Z" fill="#000"/>
    <path d="m1358 1694-12-40c0-2 0-3 2-3l6-2a2 2 0 01 2 2l10 40a2 2 0 01 0 3l-6 2c-1 0-2 0-2-2Z" fill="#fff"/>
    <path d="m1345 1654 12 40c0 3 2 4 4 4l5-2c2 0 3-2 2-4l-10-42c0-2-3-3-5-3l-6 2c-2 0-3 3-2 5Zm20 38-6 2-11-40 7-3Z" fill="#000"/>
    <path d="m1346 1698-12-41a2 2 0 01 2-3l6-2a2 2 0 01 2 2l12 41s0 2-2 2l-5 2-3-1Z" fill="#fff"/>
    <path d="m1333 1657 12 41c0 2 3 3 4 3l6-2 2-4-12-41c0-3-2-4-4-4l-6 2a4 4 0 00-2 5Zm21 38-7 2-12-40 8-2Z" fill="#000"/>
    <path d="m1335 1701-13-41 1-2 6-2h3l12 42v2l-7 2h-2Z" fill="#fff"/>
    <path d="m1320 1660 14 41c0 3 2 4 4 3l6-1c1-1 2-3 1-5l-12-41c0-2-3-4-4-3l-6 2c-2 0-3 3-2 5Zm23 38-7 2-13-40 8-2Z" fill="#000"/>
    <path d="m1324 1705-14-40a2 2 0 01 1-3l6-2 3 1 13 40a2 2 0 01-1 3l-6 2a2 2 0 01-2-1Z" fill="#fff"/>
    <path d="m1309 1665 14 40c0 2 3 3 4 3l6-2c1 0 2-3 0-5l-12-40c0-3-2-4-4-4l-7 3c-2 1-2 3-1 5Zm23 37-7 2-14-40 8-2Z" fill="#000"/>
    <path d="m1313 1709-15-40a2 2 0 01 1-3l6-2 3 1 14 40-1 3-6 1h-2Z" fill="#fff"/>
    <path d="m1297 1670 15 40c0 1 2 3 4 2l6-2c1 0 2-3 0-5l-13-40c0-2-3-3-5-3l-6 3c-1 0-2 3-1 4Zm24 36-7 2-15-40 8-2Z" fill="#000"/>
    <path d="m1302 1713-16-40 1-2 6-3a2 2 0 01 3 1l15 40a2 2 0 01-1 3l-6 2h-2Z" fill="#fff"/>
    <path d="m1285 1674 16 40c0 2 3 3 4 3l6-3c1 0 2-2 0-4l-14-40c0-2-3-4-5-3l-6 2c-1 1-2 3-1 5Zm25 36-7 3-15-40 7-2Z" fill="#000"/>
    <path d="m1291 1718-17-40a2 2 0 01 1-2l6-3 3 1 16 40v3l-6 2a2 2 0 01-3-1Z" fill="#fff"/>
    <path d="m1273 1679 17 40 4 2 6-3c1 0 2-2 0-4l-15-40c0-2-3-4-5-3l-6 3a4 4 0 00 0 5Zm26 35-7 3-16-39 7-3Z" fill="#000"/>
    <path d="m1280 1722-18-38a2 2 0 01 2-3l5-2a2 2 0 01 3 0l17 40a2 2 0 01 0 2l-6 2h-2Z" fill="#fff"/>
    <path d="m1261 1684 19 39c0 2 3 3 4 2l5-2c2 0 2-3 1-5l-17-40-4-2-7 3a4 4 0 00 0 5Zm27 35-6 2-18-37 7-4Z" fill="#000"/>
    <path d="m1270 1728-20-39a2 2 0 01 1-2l6-3h3l18 40v2l-6 2a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1250 1690 20 38c1 2 3 3 4 2l6-2v-5l-18-39c0-2-3-3-5-2l-6 3c-1 0-2 3-1 5Zm28 34-7 3-18-38 7-3Z" fill="#000"/>
    <path d="m1260 1733-20-38v-2l7-3a2 2 0 01 2 0l20 39a2 2 0 01 0 2l-6 3a2 2 0 01-3-1Z" fill="#fff"/>
    <path d="m1239 1696 20 37c0 2 3 3 5 3l5-3v-5L1250 1690a3 3 0 00-5-2l-6 3a4 4 0 00 0 5Zm28 33-6 3-20-37 7-4Z" fill="#000"/>
    <path d="m1250 1739-22-37a2 2 0 01 0-3l7-3a2 2 0 01 2 0l21 38v2l-6 3h-2Z" fill="#fff"/>
    <path d="m1227 1702 22 37c1 2 3 3 5 2l5-3v-5l-20-37c-2-2-4-3-6-2l-5 3a4 4 0 00 0 5Zm30 32-6 4-21-37 7-3Z" fill="#000"/>
    <path d="m1240 1745-23-37a2 2 0 01 0-2l7-4h2l22 37v3l-5 3a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1216 1710 23 35c2 2 3 3 5 2l5-3v-5l-22-37c0-2-3-2-5-1l-6 3a4 4 0 00 0 5Zm31 30-6 4-22-36 7-4Z" fill="#000"/>
    <path d="m1230 1751-23-35v-3l6-4h2l23 37v2l-5 3h-2Z" fill="#fff"/>
    <path d="m1206 1716 24 36c0 1 3 2 4 1l5-3v-5l-23-36c0-2-3-2-5-2l-5 4a4 4 0 00 0 5Zm31 30-6 4-23-35 6-4Z" fill="#000"/>
    <path d="m1221 1758-25-35a2 2 0 01 0-3l6-4h2l25 36v2l-5 4a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1195 1724 25 34c2 2 4 3 5 2l5-4v-4l-24-36a4 4 0 00-5-1l-6 4v5Zm33 28-6 5-25-34 7-5Z" fill="#000"/>
    <path d="m1212 1764-26-33v-3l5-4h3l25 35v2l-5 4h-2Z" fill="#fff"/>
    <path d="m1185 1731 26 34c1 2 3 3 5 2l4-4c2-1 2-3 0-5l-25-34a4 4 0 00-5-2l-5 4c-2 2-2 4 0 5Zm33 28-5 5-26-34 6-5Z" fill="#000"/>
    <path d="m1203 1772-27-33a2 2 0 01 0-3l5-4a2 2 0 01 2 0l27 34a2 2 0 01 0 2l-5 4h-2Z" fill="#fff"/>
    <path d="m1175 1740 27 33c1 1 3 2 5 1l4-3c2-2 1-4 0-5l-26-34a4 4 0 00-5-1l-5 4c-2 1-2 4 0 5Zm34 26-5 5-27-32 6-5Z" fill="#000"/>
    <path d="m1194 1780-28-33v-2l5-5h2l28 33a2 2 0 01 0 3l-4 3h-3Z" fill="#fff"/>
    <path d="m1165 1748 28 32c2 2 4 2 5 1l5-4v-4l-28-33a4 4 0 00-6-1l-5 4v5Zm35 26-5 4-28-31 6-5Z" fill="#000"/>
    <path d="m1186 1787-30-30a2 2 0 01 0-4l5-4h2l29 32a2 2 0 01 0 2l-4 4h-2Z" fill="#fff"/>
    <path d="m1155 1757 30 30c2 2 4 3 5 2l4-4c2-1 1-3 0-5l-29-32a4 4 0 00-5 0l-5 4v5Zm36 24-5 5-28-30 5-5Z" fill="#000"/>
    <path d="m1178 1795-30-30c-1 0-1-2 0-2l4-5a2 2 0 01 3 0l30 30v3l-4 4a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1146 1766 30 30c2 2 4 2 5 0l4-3v-5l-30-31a4 4 0 00-6 0l-4 4a4 4 0 00 1 5Zm37 23-5 5-30-30 6-5Z" fill="#000"/>
    <path d="m1170 1804-31-30a2 2 0 01 0-2l4-5a2 2 0 01 3 0l30 30c1 0 1 2 0 2l-3 5h-3Z" fill="#fff"/>
    <path d="m1137 1775 32 30h5l4-4-1-5-30-30a4 4 0 00-5 0c-2 0-4 3-5 4a4 4 0 00 0 5Zm38 22-5 6-30-30 5-5Z" fill="#000"/>
    <path d="m1162 1812-32-28v-3l4-5a2 2 0 01 3 0l32 30v2l-4 4a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1129 1785 32 28c2 1 4 2 5 0l4-4-1-5-32-29a4 4 0 00-5 0l-4 5v5Zm38 20-5 6-31-28 5-6Z" fill="#000"/>
    <path d="m1154 1821-33-27v-3l5-5h2l32 28v3l-3 4h-3Z" fill="#fff"/>
    <path d="m1120 1795 33 27c2 1 4 2 5 0l4-4-1-5-33-28a4 4 0 00-5 0l-4 5c-1 2 0 4 1 5Zm39 20-5 5-32-27 5-6Z" fill="#000"/>
    <path d="m1147 1830-34-27v-2l4-5a2 2 0 01 3 0l33 27v3l-4 4a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1112 1804 34 27c2 1 4 2 5 0l4-5s0-3-2-4l-33-27a4 4 0 00-5 0l-4 5c-1 1 0 4 1 4Zm40 20-5 5-33-26 5-6Z" fill="#000"/>
    <path d="m1140 1840-34-26v-3l4-5h3l33 26v3l-4 4c0 1-1 1-2 0Z" fill="#fff"/>
    <path d="m1105 1815 34 25c2 2 4 2 5 0l3-4c1-2 0-4-1-5l-34-27a4 4 0 00-5 0l-4 6a4 4 0 00 2 5Zm40 17-5 6-33-25 5-6Z" fill="#000"/>
    <path d="m1133 1849-35-25a2 2 0 01 0-3l4-5h2l35 26v2l-4 5h-3Z" fill="#fff"/>
    <path d="m1097 1825 35 25h5l3-5c1-1 0-3-1-5l-34-25a4 4 0 00-6 0l-3 5c-1 2 0 4 1 5Zm40 17-4 6-34-25 4-6Z" fill="#000"/>
    <path d="m1126 1859-35-25v-3l3-5a2 2 0 01 3 0l35 25v2l-4 5a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1090 1835 35 25h5l3-5c1-1 0-3-1-5l-35-25a4 4 0 00-5 0l-4 6c0 1 0 3 2 4Zm40 16-4 6-35-24 5-6Z" fill="#000"/>
    <path d="m1119 1868-36-24v-2l4-6h3l35 24v3l-4 5a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1083 1846 35 23c2 2 4 2 5 0l4-5s0-3-2-4l-35-25a4 4 0 00-5 0l-4 5c0 2 0 4 2 6Zm40 15-4 6-35-23 5-7Z" fill="#000"/>
    <path d="m1112 1878-35-23a2 2 0 01-1-3l4-5a2 2 0 01 2 0l36 23v3l-4 5h-2Z" fill="#fff"/>
    <path d="m1076 1856 36 23c1 2 3 2 5 0l3-5c0-1 0-3-2-5l-35-23a4 4 0 00-5 0l-4 5c0 2 0 4 2 5Zm40 15-3 6-35-23 4-6Z" fill="#000"/>
    <path d="m1106 1888-36-23v-2l3-6a2 2 0 01 3 0l36 23v3l-3 5a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1070 1866 35 23c2 2 4 2 5 0l3-5c1-1 0-4-1-4l-36-24a4 4 0 00-5 0l-4 6a4 4 0 00 2 4Zm40 15-4 6-35-22 4-7Z" fill="#000"/>
    <path d="m1100 1899-36-23v-3l2-5a2 2 0 01 3 0l36 22a2 2 0 01 0 3l-3 5a2 2 0 01-2 0Z" fill="#fff"/>
    <path d="m1063 1877 36 23h5l3-6c0-1 0-3-2-4l-36-23a4 4 0 00-5 0l-3 5c-1 2 0 4 2 5Zm40 14-3 6-36-22 4-6Z" fill="#000"/>
    <path d="m1093 1909-36-23c-1 0-2-1-1-2l3-5c0-1 2-2 3-1l36 23a2 2 0 01 0 2l-2 5a2 2 0 01-3 0Z" fill="#fff"/>
    <path d="m1056 1888 37 22h5l3-5a4 4 0 00-2-5l-37-23a4 4 0 00-5 0l-3 6c0 2 0 4 2 4Zm41 13-4 7-35-22 4-7Z" fill="#000"/>
    <path d="m1087 1920-37-23a2 2 0 01 0-3l3-5a2 2 0 01 2 0l37 22a2 2 0 01 0 2l-3 6h-2Z" fill="#fff"/>
    <path d="m1050 1898 37 22h5l3-6s0-3-2-4l-37-23a4 4 0 00-5 1l-3 5c0 2 0 4 2 5Zm41 13-3 7-36-22 4-6Z" fill="#000"/>
    <path d="m1043 1905 3-5h3l37 21v3l-3 5h-3l-36-21v-3Z" fill="#fff"/>
    <path d="m1043 1919 37 22h5l3-6c0-1 0-3-2-4l-37-22a4 4 0 00-5 0l-3 5c0 2 0 4 2 5Zm42 13-4 6-36-21 4-7Z" fill="#000"/>
  </g>
  <path d="M1560 661h-10a4 4 0 01-3-3V549a4 4 0 01 4-4h8c2 0 3 2 3 4l1 109a4 4 0 01-4 3Z" fill="#231f20"/>
  <path d="M1558 657h-6a2 2 0 01-2-2V552c0-1 0-2 2-2h6a2 2 0 01 2 2v103a2 2 0 01-2 2Z" fill="#fff"/>
  <path d="M1575 661h-8a4 4 0 01-4-3v-110a4 4 0 01 3-3h8a4 4 0 01 4 4v109c0 2 0 3-3 3Z" fill="#231f20"/>
  <path d="M1574 657h-6l-2-2-1-103a2 2 0 01 2-2h6a2 2 0 01 2 2l1 103s0 2-2 2Z" fill="#fff"/>
  <path d="M1590 661h-8a4 4 0 01-3-3l-1-110a4 4 0 01 3-3h9c2 0 4 2 4 4v108a4 4 0 01-3 4Z" fill="#231f20"/>
  <path d="M1590 657h-6c-2 0-3-1-3-2V552c0-1 0-2 2-2h6a2 2 0 01 2 2l1 103a2 2 0 01-2 2Z" fill="#fff"/>
  <path d="M1606 661h-8a4 4 0 01-4-4V550c0-2 1-4 3-4h9a4 4 0 01 3 4v108c0 2-1 4-3 4Z" fill="#231f20"/>
  <path d="M1605 657h-6l-2-2-1-103c0-2 0-2 2-2h7a2 2 0 01 2 2v103s0 2-2 2Z" fill="#fff"/>
  <path d="M1621 661h-8a4 4 0 01-3-4V550c0-2 1-4 3-4h9a4 4 0 01 4 4v108a4 4 0 01-5 4Z" fill="#231f20"/>
  <path d="M1620 657h-6a2 2 0 01-2-3V552c0-2 0-3 2-3h7a2 2 0 01 2 3v103a2 2 0 01-3 2Z" fill="#fff"/>
  <path d="M1636 661h-8a3 3 0 01-3-3v-110a4 4 0 01 4-3h9c2 0 4 2 4 4l-2 109a3 3 0 01-4 3Z" fill="#231f20"/>
  <path d="M1635 657h-6a2 2 0 01-2-2V552l3-2h7s2 0 2 2l-2 103c0 1 0 2-2 2Z" fill="#fff"/>
  <path d="M1651 661h-8a4 4 0 01-3-3l2-109a4 4 0 01 3-3h10a4 4 0 01 3 3l-4 109c0 2-1 3-3 3Z" fill="#231f20"/>
  <path d="M1650 657h-6a2 2 0 01-2-2l2-103c0-1 0-2 2-2h7c1 0 2 0 2 2l-3 103a2 2 0 01-2 2Z" fill="#fff"/>
  <path d="M1666 662h-8a4 4 0 01-4-4l4-108c0-3 2-4 4-4h9c2 0 4 2 3 4l-5 108a4 4 0 01-3 4Z" fill="#231f20"/>
  <path d="M1665 658h-6a2 2 0 01-2-3l3-103 3-2h6c2 0 3 1 3 3l-5 102a2 2 0 01-2 3Z" fill="#fff"/>
  <path d="M1680 663h-8a3 3 0 01-3-4l6-109a4 4 0 01 4-3h9c2 0 3 2 3 4l-7 108a4 4 0 01-4 4Z" fill="#231f20"/>
  <path d="M1680 658h-7a2 2 0 01-2-2l6-103a2 2 0 01 2-2h7a2 2 0 01 2 3l-7 102a2 2 0 01-2 2Z" fill="#fff"/>
  <path d="M1694 664h-8a4 4 0 01-2-5l7-108a4 4 0 01 4-3h10a4 4 0 01 3 4l-10 108a3 3 0 01-4 4Z" fill="#231f20"/>
  <path d="M1694 660h-6a2 2 0 01-2-3l8-102c0-2 0-3 2-3l7 1a2 2 0 01 2 2l-10 103-1 1Z" fill="#fff"/>
  <path d="M1708 665h-7a4 4 0 01-3-5l10-108a4 4 0 01 4-3l10 1a4 4 0 01 3 4L1710 662c0 2-1 3-3 3Z" fill="#231f20"/>
  <path d="M1708 660h-6a2 2 0 01-2-2l10-103s1-2 3-1h7a2 2 0 01 2 3L1710 659a2 2 0 01-3 2Z" fill="#fff"/>
  <path d="M1721 667h-7a4 4 0 01-3-5l14-108a4 4 0 01 5-3l9 2c2 0 4 2 3 4l-17 107a3 3 0 01-4 3Z" fill="#231f20"/>
  <path d="m1721 663-5-1a2 2 0 01-2-3l13-102a2 2 0 01 3-1h7a2 2 0 01 2 3l-15 102a2 2 0 01-3 2Z" fill="#fff"/>
  <path d="m1735 670-7-2a3 3 0 01-3-4l17-107c0-2 3-4 5-3l10 2a4 4 0 01 3 4l-22 106c0 2-2 4-3 3Z" fill="#231f20"/>
  <path d="m1735 665-6-1a2 2 0 01-1-3l17-101a2 2 0 01 2-2l7 2a2 2 0 01 2 2l-20 101-1 2Z" fill="#fff"/>
  <path d="m1747 672-6-1a3 3 0 01-3-5L1760 560c0-2 3-3 5-3l10 2a4 4 0 01 3 5L1750 669a3 3 0 01-4 3Z" fill="#231f20"/>
  <path d="m1748 668-6-2a2 2 0 01 0-2l20-100c0-2 2-3 3-2l7 1 2 3-24 100-2 2Z" fill="#fff"/>
  <path d="m1760 675-6-1a3 3 0 01-3-5l27-105a4 4 0 01 5-3l10 3c2 0 3 3 3 5l-32 104a3 3 0 01-4 2Z" fill="#231f20"/>
  <path d="M1760 670h-5a2 2 0 01-1-3l25-100a2 2 0 01 3-2l8 3s2 1 1 2l-29 99c0 1 0 2-2 2Z" fill="#fff"/>
  <path d="m1771 679-6-2a3 3 0 01-2-4l32-104c0-2 3-3 5-2 4 0 7 2 10 3 3 0 4 3 3 5l-37 102c-2 2-3 3-5 2Z" fill="#231f20"/>
  <path d="m1772 675-5-2V670l30-98a2 2 0 01 3 0c3 0 5 0 7 2 2 0 3 1 2 2l-35 97-2 2Z" fill="#fff"/>
  <path d="m1782 683-6-2a3 3 0 01 0-4l37-102c0-2 3-3 5-2l10 4a4 4 0 01 3 5l-45 100s-2 2-4 1Z" fill="#231f20"/>
  <path d="m1783 679-4-2a2 2 0 01-1-2l37-96c0-2 2-2 3-2l7 3 2 3-41 95h-3Z" fill="#fff"/>
  <path d="m1793 688-6-3a3 3 0 01 0-4l44-99a4 4 0 01 5-1l10 4a4 4 0 01 3 6l-52 95a3 3 0 01-4 2Z" fill="#231f20"/>
  <path d="m1794 684-4-2c-1 0-2-2-1-3l43-93c0-1 3-2 4 0l7 3c1 0 2 2 1 3l-48 90c0 2-1 2-2 2Z" fill="#fff"/>
  <path d="m1803 694-6-3a3 3 0 01 0-5l51-95c2-2 4-3 6-1l10 5a4 4 0 01 1 5l-58 92c0 2-3 2-4 2Z" fill="#231f20"/>
  <path d="m1804 690-4-3a2 2 0 01 0-2l50-90a2 2 0 01 3 0l7 3c1 0 2 2 1 3l-54 88-3 1Z" fill="#fff"/>
  <path d="m1812 700-5-4a3 3 0 01 0-4l58-92a4 4 0 01 6 0l8 5c2 1 3 4 2 6l-65 87c0 2-2 3-4 2Z" fill="#231f20"/>
  <path d="m1814 696-4-3a2 2 0 01 0-2l56-86a2 2 0 01 3 0l7 4c1 0 2 2 0 3l-60 84h-2Z" fill="#fff"/>
  <path d="m1821 707-4-4a3 3 0 01 0-4L1880 610h6l8 7c2 1 2 4 1 6l-70 82a3 3 0 01-5 1Z" fill="#231f20"/>
  <path d="m1824 703-4-3a2 2 0 01 0-3l62-82a2 2 0 01 3 0l6 5a2 2 0 01 1 3l-66 80h-2Z" fill="#fff"/>
  <path d="m1830 715-5-4a3 3 0 01 0-5l70-83a4 4 0 01 6 0l8 7c2 2 2 4 0 6l-75 78c-1 1-3 1-4 0Z" fill="#231f20"/>
  <path d="m1833 711-4-3a2 2 0 01 0-3l67-78h4l6 6v3l-70 75h-3Z" fill="#fff"/>
  <path d="m1838 723-4-4v-5l76-78h6l7 8a4 4 0 01 0 6l-80 73h-5Z" fill="#231f20"/>
  <path d="m1841 720-3-4c-1 0-1-2 0-2L1910 640a2 2 0 01 4 0l5 5v4l-76 70h-2Z" fill="#fff"/>
  <path d="m1846 732-4-5c-1-1 0-3 0-4l82-73c2-1 4 0 5 1l7 8c2 2 2 4 0 6l-86 67h-4Z" fill="#231f20"/>
  <path d="m1849 729-3-4v-3l78-68h3l5 7v4l-80 64a2 2 0 01-3 0Z" fill="#fff"/>
  <path d="m1853 741-4-5 1-4 86-67c2-1 5 0 6 1l6 9a4 4 0 01 0 6l-90 60a3 3 0 01-5 0Z" fill="#231f20"/>
  <path d="m1856 738-3-4V730l83-62a2 2 0 01 3 1l5 6a2 2 0 01 0 4l-85 58h-3Z" fill="#fff"/>
  <path d="m1859 750-3-5c-1-1 0-3 1-4l90-60c2 0 5 0 6 2l5 10c2 0 1 3 0 5l-95 53c-1 1-3 0-4 0Z" fill="#231f20"/>
  <path d="m1862 748-2-4V740l87-56a3 3 0 01 3 1l4 7a2 2 0 01 0 3l-89 52a2 2 0 01-3 0Z" fill="#fff"/>
  <path d="m1864 760-3-5c0-2 0-3 2-4l95-54c2 0 4 0 5 3l5 9c1 2 0 5-1 6l-98 46h-5Z" fill="#231f20"/>
  <path d="m1868 758-2-4V750l90-50c2 0 3 0 4 2l4 7a2 2 0 01-1 3l-92 45h-3Z" fill="#fff"/>
  <path d="m1869 770-3-5c0-1 0-3 3-4l98-46c2 0 4 0 5 2l4 10a4 4 0 01-1 6l-102 40c-1 0-3 0-4-2Z" fill="#231f20"/>
  <path d="m1873 769-2-5v-2l95-43 3 1 3 8-1 3-96 38h-2Z" fill="#fff"/>
  <path d="m1873 782-2-6c0-2 0-3 2-4l102-40c2 0 4 2 5 4l3 10c0 2 0 4-2 5l-104 32c-2 1-3 0-4-1Z" fill="#231f20"/>
  <path d="m1877 780-2-5a2 2 0 01 2-2l96-36a2 2 0 01 3 2l3 7a2 2 0 01-2 3l-98 32-2-1Z" fill="#fff"/>
  <path d="m1876 793-2-6c0-2 1-3 3-4l104-32a4 4 0 01 5 3l2 10c0 2 0 4-2 5L1880 796a3 3 0 01-5-3Z" fill="#231f20"/>
  <path d="m1880 791-1-5 1-2 99-30s2 0 2 2l3 7a2 2 0 01-2 3l-100 26h-2Z" fill="#fff"/>
  <path d="m1880 806-2-7c0-2 0-3 3-4l105-26a4 4 0 01 4 3l3 10c0 2-1 4-3 4l-106 22c-2 0-4 0-5-2Z" fill="#231f20"/>
  <path d="M1883 804V800c-1-1 0-2 0-2l100-25c2 0 3 0 4 2l1 7-1 3-100 21a2 2 0 01-4 0Z" fill="#fff"/>
  <path d="m1882 819-1-7c0-2 0-4 3-4l106-22a4 4 0 01 4 4l2 9a4 4 0 01-3 5l-107 18a3 3 0 01-4-3Z" fill="#231f20"/>
  <path d="m1886 817-1-5c0-1 0-2 2-2l100-20c2 0 3 0 3 2l1 7-1 2-102 18s-2 0-2-2Z" fill="#fff"/>
  <path d="m1884 833-1-8a3 3 0 01 3-3l107-18a4 4 0 01 4 3l2 9c0 2-1 4-3 4l-108 16c-2 0-4-1-4-3Z" fill="#231f20"/>
  <path d="M1888 831v-5a2 2 0 01 1-3l102-16a2 2 0 01 2 2l1 6a2 2 0 01-2 3l-101 15a2 2 0 01-3-2Z" fill="#fff"/>
  <path d="m1886 847-1-7a3 3 0 01 3-4l108-16a4 4 0 01 4 3v10a4 4 0 01-2 4L1890 850a3 3 0 01-4-3Z" fill="#231f20"/>
  <path d="M1890 846V840a2 2 0 01 1-3l102-14a2 2 0 01 2 2l1 7a2 2 0 01-2 3l-102 12-2-1Z" fill="#fff"/>
  <path d="m1888 861-1-7c0-2 1-4 3-4l108-13c2 0 4 2 4 4l1 9c0 2-1 4-3 4l-108 10a3 3 0 01-4-3Z" fill="#231f20"/>
  <path d="M1892 860v-6a2 2 0 01 1-2l102-12c1 0 2 0 3 2v7c0 1 0 2-2 2l-102 10-2-1Z" fill="#fff"/>
  <path d="M1889 875v-7c-1-2 0-4 2-4l108-10a4 4 0 01 4 3l1 10a4 4 0 01-3 4l-108 8a4 4 0 01-4-4Z" fill="#231f20"/>
  <path d="M1893 874v-6a2 2 0 01 2-2l102-10a2 2 0 01 3 3v7s0 2-2 2l-102 8a2 2 0 01-3-2Z" fill="#fff"/>
  <path d="M1890 890v-8c0-2 0-3 3-3L2000 870c2 0 4 2 4 4v9a4 4 0 01-3 4l-108 5a4 4 0 01-4-3Z" fill="#231f20"/>
  <path d="M1894 888v-5a2 2 0 01 2-3l102-7a2 2 0 01 3 2v7c0 2-1 3-2 3l-103 5c-1 0-2 0-2-2Z" fill="#fff"/>
  <path d="M1890 904v-8c0-2 2-3 4-3l108-6a4 4 0 01 4 4v10a4 4 0 01-4 3l-108 3a4 4 0 01-4-3Z" fill="#231f20"/>
  <path d="M1894 903v-6c0-1 1-2 3-2l102-5a2 2 0 01 3 2v7l-3 3-102 3a2 2 0 01-3-2Z" fill="#fff"/>
  <path d="M1890 918V910a3 3 0 01 4-3l109-3c2 0 3 2 3 4v10a4 4 0 01-3 3h-109a4 4 0 01-3-3Z" fill="#231f20"/>
  <path d="M1895 917v-6s0-2 2-2l103-2c1 0 2 0 2 2v7a2 2 0 01-2 2l-103 1a2 2 0 01-2-2Z" fill="#fff"/>
  <path d="M1890 932v-7c0-2 2-4 4-4h108a4 4 0 01 4 4v10c0 1-2 3-4 3l-108-2a4 4 0 01-4-4Z" fill="#231f20"/>
  <path d="M1895 931v-6s0-2 2-1H2000a2 2 0 01 2 2v7c0 1 0 2-2 2l-103-2a2 2 0 01-2-2Z" fill="#fff"/>
  <path d="M1890 946V940c0-2 2-3 4-3l108 2a4 4 0 01 4 4v9c0 2-2 4-4 4l-108-5a4 4 0 01-4-4Z" fill="#231f20"/>
  <path d="M1894 946V940a2 2 0 01 3-2l103 2a2 2 0 01 2 2v8c0 1 0 2-2 2l-103-4a2 2 0 01-3-2Z" fill="#fff"/>
  <path d="M1890 960v-7a3 3 0 01 4-3l108 5c2 0 4 2 4 4v9c-1 2-3 3-5 3l-108-7a3 3 0 01-4-3Z" fill="#231f20"/>
  <path d="M1894 960v-6a2 2 0 01 2-2l103 5c1 0 2 1 2 3v6a2 2 0 01-2 3l-103-7a2 2 0 01-2-2Z" fill="#fff"/>
  <path d="M1888 975v-8a4 4 0 01 5-3l108 7a4 4 0 01 3 5v9a4 4 0 01-4 3l-108-10a4 4 0 01-4-3Z" fill="#231f20"/>
  <path d="M1893 974V970c0-2 0-3 2-2l103 7 2 2v7a2 2 0 01-3 2l-103-9a2 2 0 01-1-2Z" fill="#fff"/>
  <path d="M1887 989V980c1-1 3-3 5-3l108 10c2 0 3 3 3 5v9c0 2-3 3-5 3L1890 993a3 3 0 01-3-4Z" fill="#231f20"/>
  <path d="M1891 989v-6l3-2 102 10 2 2v7a2 2 0 01-3 2l-102-10a2 2 0 01-2-3Z" fill="#fff"/>
  <path d="m1885 1003 1-8c0-1 2-3 4-2l108 12a4 4 0 01 3 4v10c-1 1-3 3-5 3l-108-15a4 4 0 01-3-4Z" fill="#231f20"/>
  <path d="M1890 1003v-6l3-2 102 12a2 2 0 01 2 3v7a2 2 0 01-3 2l-102-14a2 2 0 01-2-2Z" fill="#fff"/>
  <path d="m1883 1017 1-7c0-2 3-4 4-3l108 15a4 4 0 01 3 4l-1 10a4 4 0 01-4 3l-108-18a4 4 0 01-3-4Z" fill="#231f20"/>
  <path d="M1888 1017v-6l3-2 102 15s2 1 2 3l-1 7a2 2 0 01-3 1L1890 1020a2 2 0 01-2-2Z" fill="#fff"/>
  <path d="m1880 1030 2-6c0-2 3-4 4-3l108 18c2 0 3 2 2 4l-1 10a4 4 0 01-5 2l-106-20a4 4 0 01-3-4Z" fill="#231f20"/>
  <path d="m1885 1030 1-5c0-1 2-2 3-1L1990 1040l2 2-2 7c0 1 0 2-2 2l-101-19a2 2 0 01-2-2Z" fill="#fff"/>
  <path d="m1878 1045 1-8c0-2 3-3 5-2l106 20a4 4 0 01 3 4l-2 10c0 1-2 3-4 2l-106-23a4 4 0 01-3-3Z" fill="#231f20"/>
  <path d="M1883 1045v-6a2 2 0 01 3-2l101 20a2 2 0 01 2 3l-2 7a2 2 0 01-2 2l-100-22a2 2 0 01-3-2Z" fill="#fff"/>
  <path d="m1875 1058 2-7a3 3 0 01 4-2l106 23a4 4 0 01 3 5l-2 8c0 3-3 4-5 4l-105-27a3 3 0 01-3-4Z" fill="#231f20"/>
  <path d="M1880 1059v-6l4-2 100 23a2 2 0 01 1 3l-2 6c0 2-1 2-2 2l-100-24-2-2Z" fill="#fff"/>
  <path d="m1871 1072 2-7c0-2 3-3 5-3l105 27c2 0 3 2 3 4l-3 10a4 4 0 01-4 2l-105-29a4 4 0 01-3-4Z" fill="#231f20"/>
  <path d="m1876 1072 1-6h3l100 24c1 0 2 2 1 3l-1 7s-2 2-3 1l-100-27a2 2 0 01-1-2Z" fill="#fff"/>
  <path d="m1868 1085 2-7a3 3 0 01 4-2l105 29a4 4 0 01 2 5l-2 9c0 2-3 3-5 2L1870 1090a3 3 0 01-2-4Z" fill="#231f20"/>
  <path d="m1872 1086 2-6a2 2 0 01 2-1l100 28a2 2 0 01 0 3l-1 6a2 2 0 01-3 2l-98-30a2 2 0 01-2-2Z" fill="#fff"/>
  <path d="m1863 1098 3-7 4-2 104 32a4 4 0 01 2 5l-3 10-4 2-103-35a4 4 0 01-3-5Z" fill="#231f20"/>
  <path d="m1868 1099 2-5c0-2 1-2 2-2l98 31a2 2 0 01 2 3l-2 7a2 2 0 01-3 1l-98-33a2 2 0 01-1-2Z" fill="#fff"/>
  <path d="m1860 1112 1-7a3 3 0 01 5-2l103 35a4 4 0 01 2 5l-3 8c0 3-3 4-5 3l-102-38a3 3 0 01-2-4Z" fill="#231f20"/>
  <path d="m1864 1112 1-5c0-1 2-2 3-1l97 33c1 0 2 2 1 3l-2 7a2 2 0 01-3 1l-96-35a2 2 0 01-1-3Z" fill="#fff"/>
  <path d="m1854 1124 3-7c0-1 3-2 4-1l102 38a4 4 0 01 2 5l-4 9c0 2-2 3-5 2l-100-41a4 4 0 01-2-5Z" fill="#231f20"/>
  <path d="m1858 1125 2-5a2 2 0 01 3-1l96 37a2 2 0 01 1 3l-3 6c0 1-1 2-3 2l-94-40a2 2 0 01-2-2Z" fill="#fff"/>
  <path d="m1849 1137 2-7a3 3 0 01 5-1l100 41a4 4 0 01 2 5l-4 9a4 4 0 01-5 2l-98-45a4 4 0 01-2-4Z" fill="#231f20"/>
  <path d="m1853 1138 2-5a2 2 0 01 3-1l95 40v3l-2 6a2 2 0 01-3 2l-94-42a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1843 1150 3-7c1-2 3-2 5-2l99 45a4 4 0 01 1 5l-4 9c0 2-3 3-5 2l-97-48a3 3 0 01-2-5Z" fill="#231f20"/>
  <path d="m1847 1150 3-5h3l93 42a2 2 0 01 0 3l-2 6c0 2-2 2-3 2l-93-45a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1837 1161 3-6c1-2 3-2 5-1l97 48c2 0 3 3 2 5l-5 8a4 4 0 01-5 2l-95-50a3 3 0 01-2-6Z" fill="#231f20"/>
  <path d="m1841 1163 3-5c0-1 1-2 2-1l92 46a2 2 0 01 1 3l-3 6c0 2-2 2-3 2l-90-49a2 2 0 01-2-2Z" fill="#fff"/>
  <path d="m1830 1173 4-6c0-1 3-2 4 0l96 50a4 4 0 01 1 6l-4 8c-1 2-4 3-6 2l-94-55a3 3 0 01-1-5Z" fill="#231f20"/>
  <path d="m1835 1175 3-5a2 2 0 01 2 0l90 48a2 2 0 01 1 4l-4 6h-3l-89-50a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1823 1185 4-6c1-2 3-2 5-1l94 55c1 0 2 3 0 5l-4 8a4 4 0 01-5 2l-92-59a4 4 0 01-2-4Z" fill="#231f20"/>
  <path d="m1828 1186 2-4a2 2 0 01 3 0l89 52v3l-3 6c0 1-2 2-3 1l-88-54a2 2 0 01 0-4Z" fill="#fff"/>
  <path d="m1816 1196 4-6c1-1 3-2 5 0l92 58c1 0 2 3 0 5l-5 8c0 2-3 2-5 1l-90-61a3 3 0 01 0-5Z" fill="#231f20"/>
  <path d="m1820 1198 3-5a2 2 0 01 3 0l86 56c1 0 2 2 0 3l-3 6c0 1-2 2-3 0l-85-57a2 2 0 01-1-3Z" fill="#fff"/>
  <path d="m1809 1207 4-6c0-1 3-2 4 0l90 61a4 4 0 01 0 6l-5 7a4 4 0 01-6 2L1810 1210a3 3 0 01-1-4Z" fill="#231f20"/>
  <path d="m1812 1209 4-5h3l84 60a2 2 0 01 0 2l-4 6a2 2 0 01-4 0l-82-60a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1800 1218 5-6c1-2 3-2 5 0l86 65c2 1 2 4 0 6l-5 7c-1 2-4 2-5 1l-85-69a3 3 0 01 0-5Z" fill="#231f20"/>
  <path d="m1805 1220 3-5h3l82 63v3l-5 6h-3l-80-64a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1792 1227 5-5c0-1 3-1 4 0l85 69a4 4 0 01 0 5l-7 8a4 4 0 01-5 0l-81-72a3 3 0 01-1-5Z" fill="#231f20"/>
  <path d="m1796 1230 3-4a2 2 0 01 3 0l80 65v4l-5 5a2 2 0 01-3 0l-78-67v-3Z" fill="#fff"/>
  <path d="m1783 1237 5-5h5l80 72c2 2 2 4 0 6l-6 7a4 4 0 01-5 0l-78-75a4 4 0 01 0-5Z" fill="#231f20"/>
  <path d="m1787 1240 4-4h3l76 69v3l-5 5h-3l-75-70v-3Z" fill="#fff"/>
  <path d="m1774 1246 5-4h5l78 76a4 4 0 01 0 5l-7 7a4 4 0 01-6 0l-74-80a3 3 0 01-1-4Z" fill="#231f20"/>
  <path d="m1778 1249 4-4h2l74 72a2 2 0 01 0 3l-6 5h-3l-71-74v-2Z" fill="#fff"/>
  <path d="m1764 1255 5-5 5 1 75 79v6l-8 6a4 4 0 01-5 0l-71-82a4 4 0 01 0-5Z" fill="#231f20"/>
  <path d="m1768 1258 4-4h3l70 75v4l-6 5h-3l-68-78v-2Z" fill="#fff"/>
  <path d="m1754 1264 6-4a3 3 0 01 5 0l70 83a4 4 0 01 0 5l-8 6c-1 2-4 2-5 0l-68-85a3 3 0 01 0-5Z" fill="#231f20"/>
  <path d="m1758 1267 4-4h3l67 78a2 2 0 01 0 4l-6 4a2 2 0 01-4 0l-64-80v-2Z" fill="#fff"/>
  <path d="m1744 1272 6-5 4 1 68 86a4 4 0 01-1 5l-8 6a4 4 0 01-6 0l-63-88a3 3 0 01 0-5Z" fill="#231f20"/>
  <path d="m1748 1275 4-4h3l63 82a2 2 0 01 0 3l-7 5a2 2 0 01-3 0l-60-83a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1734 1280 5-5 5 1 63 88c2 2 1 5 0 6a90 90 0 01-8 5 4 4 0 01-6 0l-60-90a3 3 0 01 0-6Z" fill="#231f20"/>
  <path d="m1737 1283 4-4a2 2 0 01 3 1l60 84v3a57 57 0 01-8 5h-2l-57-86a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1722 1287 6-4 5 1 60 90c1 2 0 4-1 6l-8 5-6-1-57-93 1-4Z" fill="#231f20"/>
  <path d="m1725 1290 5-3a2 2 0 01 3 0l56 87a2 2 0 01 0 3l-6 3a2 2 0 01-4 0l-54-87a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1710 1294 7-4c1 0 4 0 4 2l58 92a4 4 0 01-2 5l-8 5c-2 1-4 0-5-1l-54-94a4 4 0 01 0-5Z" fill="#231f20"/>
  <path d="m1713 1297 5-3 3 1 54 88-1 3-6 4a2 2 0 01-3 0l-52-90v-3Z" fill="#fff"/>
  <path d="m1699 1300 6-3c2 0 4 0 5 2l54 94a4 4 0 01-2 5l-8 5c-2 0-4 0-6-2l-50-96c-1-1 0-3 0-4Z" fill="#231f20"/>
  <path d="m1701 1304 5-3 3 1 50 90a2 2 0 01 0 3l-6 3a2 2 0 01-4 0l-48-91a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1687 1307 6-3a3 3 0 01 5 1l50 96c1 2 0 5-2 6l-8 4c-2 0-4 0-5-2l-48-98a3 3 0 01 2-4Z" fill="#231f20"/>
  <path d="m1690 1310 4-3c1 0 3 0 3 2l48 90-1 4-7 3h-3l-45-93v-3Z" fill="#fff"/>
  <path d="m1674 1313 7-3c2-1 4 0 4 1l48 98c0 2 0 5-2 5l-9 4a4 4 0 01-5-1l-44-100a3 3 0 01 1-4Z" fill="#231f20"/>
  <path d="m1677 1316 5-2 3 1 44 93a2 2 0 01-1 3l-7 3-3-1-42-94v-3Z" fill="#fff"/>
  <path d="m1662 1318 7-3c1 0 3 0 4 2l44 100a4 4 0 01-3 5l-8 3c-2 1-4 0-5-2l-40-100c-1-2 0-4 1-5Z" fill="#231f20"/>
  <path d="m1664 1322 6-2 3 1 40 95v3l-7 2h-4l-38-96a2 2 0 01 0-3Z" fill="#fff"/>
  <path d="m1650 1323 7-3c1 0 3 0 4 3l40 100c0 2 0 5-2 5l-10 4c-1 0-4 0-4-2l-37-103c0-1 0-3 2-4Z" fill="#231f20"/>
  <path d="m1652 1327 5-2 3 1 37 96a2 2 0 01-1 3l-7 2a2 2 0 01-3 0l-35-98v-2Z" fill="#fff"/>
  <path d="m1637 1327 7-2a3 3 0 01 4 2l36 103a4 4 0 01-2 5l-10 3a4 4 0 01-4-3l-33-103a4 4 0 01 2-5Z" fill="#231f20"/>
  <path d="m1639 1331 5-2c1 0 2 0 3 2l34 97a2 2 0 01-2 3l-7 2a2 2 0 01-3-1l-31-98v-3Z" fill="#fff"/>
  <path d="m1624 1331 7-2c1 0 3 0 4 3l33 103a4 4 0 01-3 5l-9 3c-2 0-4 0-5-3l-30-104c0-2 0-4 3-5Z" fill="#231f20"/>
  <path d="m1626 1335 5-1 3 1 30 98-1 3-7 2a2 2 0 01-3-1l-29-100 2-2Z" fill="#fff"/>
  <path d="m1610 1335 7-2c2 0 4 0 4 3l30 104a4 4 0 01-3 5l-8 2a4 4 0 01-5-2l-27-105a4 4 0 01 2-5Z" fill="#231f20"/>
  <path d="m1612 1340 6-2 2 1 28 100s0 2-2 2l-7 2a2 2 0 01-2-2l-26-100 1-2Z" fill="#fff"/>
  <path d="m1596 1339 8-2a3 3 0 01 4 3l27 105a4 4 0 01-3 4l-9 2c-2 0-4 0-5-2l-24-106a3 3 0 01 2-4Z" fill="#231f20"/>
  <path d="m1598 1343 6-2a2 2 0 01 2 2l25 100-1 2-7 2c-1 0-2 0-3-2l-23-100a2 2 0 01 1-3Z" fill="#fff"/>
  <path d="m1583 1342 7-2a3 3 0 01 4 3l24 106c0 2 0 4-3 4l-9 2a4 4 0 01-4-3l-22-106c0-2 0-4 3-4Z" fill="#231f20"/>
  <path d="m1584 1346 6-2a2 2 0 01 3 2l22 100c0 1 0 3-2 3l-6 1-3-1-21-101 1-2Z" fill="#fff"/>
  <path d="m1569 1345 7-2c2 0 4 0 4 3l22 106c0 2-1 4-3 5l-9 1a4 4 0 01-4-3l-20-106a3 3 0 01 3-4Z" fill="#231f20"/>
  <path d="m1570 1349 6-2a2 2 0 01 2 2l20 100c0 2 0 3-2 3l-6 2c-2 0-3 0-3-2l-20-100a2 2 0 01 3-3Z" fill="#fff"/>
  <path d="m1554 1347 8-1c2 0 3 0 4 3l20 106c0 2-1 4-3 5l-10 1a4 4 0 01-3-3l-19-107c0-2 1-4 3-4Z" fill="#231f20"/>
  <path d="M1556 1351h6c0-1 2 0 2 0l19 102-2 3h-7l-3-1-17-101c0-1 0-3 2-3Z" fill="#fff"/>
  <path d="m1540 1350 7-2a3 3 0 01 4 3l18 107a4 4 0 01-3 4l-8 2a5 5 0 01-5-3l-16-107c0-2 0-4 2-5Z" fill="#231f20"/>
  <path d="m1541 1354 6-1a2 2 0 01 2 1l17 102a2 2 0 01-2 2l-6 1a2 2 0 01-3-1l-16-102c0-1 0-2 2-2Z" fill="#fff"/>
  <path d="m1525 1352 8-2c2 0 3 2 4 4l16 107a4 4 0 01-3 4l-8 1a4 4 0 01-5-3l-15-107a4 4 0 01 3-4Z" fill="#231f20"/>
  <path d="m1526 1356 6-1a2 2 0 01 3 2l15 101a2 2 0 01-2 3h-6l-2-1-15-102 1-2Z" fill="#fff"/>
  <path d="m1510 1354 8-1c2 0 4 0 4 3l16 107c0 2-1 4-3 4l-9 2a4 4 0 01-4-3l-15-108c0-2 1-4 3-4Z" fill="#231f20"/>
  <path d="M1511 1358h6a2 2 0 01 3 0l15 103-2 2-7 1s-2 0-2-2l-15-102a2 2 0 01 2-2Z" fill="#fff"/>
  <path d="m1494 1356 9-1c2 0 3 1 4 3l15 108c0 2-2 3-3 4h-9a4 4 0 01-4-2l-15-108c0-2 2-4 3-4Z" fill="#231f20"/>
  <path d="M1496 1360h6a2 2 0 01 3 1l14 102a2 2 0 01-2 2l-6 1a2 2 0 01-3-2l-14-102a2 2 0 01 2-2Z" fill="#fff"/>
  <path d="m1480 1358 8-1c2 0 3 1 4 3l14 108c0 2-1 4-3 4h-8a4 4 0 01-4-2l-15-108c0-2 1-4 3-4Z" fill="#231f20"/>
  <path d="M1480 1362h7l3 1 13 102c0 1 0 2-2 3h-6l-2-1-14-102c0-2 0-3 2-3Z" fill="#fff"/>
  <path d="M1446 1428a55 55 0 11-110 0 55 55 0 01 110 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1440 1422c4 30-15 58-42 61-26 3-50-19-54-50-4-30 15-57 41-60 27-3 51 20 55 50Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1440 1422c4 30-13 58-38 60-25 4-48-19-51-49-4-30 13-57 38-60 25-3 48 20 51 50Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1440 1422c4 30-11 57-34 60-22 3-43-20-47-50-3-30 12-57 34-60 23-2 44 20 47 50Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1440 1422c4 30-10 57-28 60-20 2-39-20-42-51-4-30 10-57 29-60 19-2 38 21 41 51Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1440 1422c4 30-7 57-23 59-16 2-33-21-36-52-4-30 7-56 23-58 16-2 33 21 36 51Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1448 1422c3 30-6 56-22 58-15 2-30-22-34-52-3-30 6-56 21-58 16-2 30 21 35 52Zm26 50-5 2a8 8 0 01-8-7 286 286 0 01-13-95c0-2 0-4 2-5 1-2 3-3 5-3a8 8 0 01 8 6l13 95c0 3 0 5-2 7Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1440 1479h-6c-5 0-8-3-10-6-10-14-17-30-19-46-3-20 1-40 11-56l3-2h5a4 4 0 01 5 3l14 101a4 4 0 01-4 6Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1450 1477h-5c-4 1-7-3-10-6-9-14-15-30-16-46a90 90 0 01 8-56l2-2h5c2 0 4 2 4 4l14 101c0 2 0 4-3 5Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m1460 1475-4 1c-2 0-5-1-6-3-10-12-14-30-16-50a135 135 0 01 4-55l3-2h5a4 4 0 01 5 3l13 101a4 4 0 01-3 5Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1472 1474h-6a5 5 0 01-5-3 291 291 0 01-12-104l3-2h5c3-1 5 0 6 3l13 101a4 4 0 01-4 5Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1486 1472h-5a4 4 0 01-6-4 580 580 0 01-12-104c0-1 1-2 3-2h5a5 5 0 01 5 3l14 101c0 3-1 5-4 6Zm-398-965h477v200h-478Zm677 566c-2-40-7-81-16-121-4-20-8-40-10-61-12-78-4-174 4-266 5-57 10-110 10-157h25c0 48-5 102-10 159-8 90-17 185-6 260l11 60c10 42 15 84 16 126" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m1387 2686 88-88m-41 96 49-49m-104-5 49-50M820 2686l88-88m-41 96 49-49m-104-5 50-50m461 578 88-88m-41 96 50-49m-104-6 48-49m-479 96 88-88m-41 96 50-49m-105-6 50-49" fill="none" stroke="#fff" stroke-linecap="round" stroke-linejoin="round" stroke-width="4"/>
  <path d="M2065 2965v13c0 10 6 16 16 18 7 1 20 1 20 11a10 10 0 01-10 11H1460s-46-1-67 7a78 78 0 00-41 55c-1 10-5 20-10 30-6 10-17 15-28 15h-330c-10 0-21-6-28-15-5-10-8-20-10-30-4-24-20-44-42-55-20-8-66-7-66-7h-630a10 10 0 01-11-10c0-10 13-10 20-12 10 0 17-9 17-18v-135c0-10-6-16-16-18-8-2-21-1-21-12a10 10 0 01 10-10h631s46 1 67-7a79 79 0 00 42-55c1-10 5-20 10-30 6-10 17-15 28-15h330c10 0 21 6 28 15 5 10 8 20 10 30 5 22 20 45 41 55 21 8 67 6 67 6h630a10 10 0 01 10 11c0 10-13 10-20 12-10 2-16 7-16 18v122Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1951 2996c-10 0-16-9-16-18v-135c0-10 6-16 16-18h130c-10 2-16 7-16 18v135c0 10 6 16 16 18Zm-1026 0c10-2 16-7 16-18v-135c0-10-6-16-16-18H218c10 2 16 7 16 18v135c0 10-7 17-16 18Zm327-88a109 109 0 11-218 0 109 109 0 01 218 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1234 2908a90 90 0 11-182 0 90 90 0 01 182 0Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1180 2908a36 36 0 11-73 0 36 36 0 01 72 0Zm-78-1786c-374 6-720 193-830 566-82 284-5 562 166 850l317-313c-147-155-318-378-268-609 58-272 368-373 615-376a60 60 0 10 0-118" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M774 2224a1043 1043 0 01-287-546 573 573 0 00-290 334 1200 1200 0 00 249 546Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <image xlink:href="data:image/png;base64, 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" width="560" height="860" x="210" y="1680"/>
  <path d="M623 1410c-77-9-155-1-230 23C180 1653 197 2010 197 2010v1c30-60 68-118 112-170A600 600 0 01 530 1670a310 310 0 01 93-260Z" fill="#fff" stroke="#000" stroke-linejoin="round" stroke-width="4"/>
  <image xlink:href="data:image/png;base64, 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" width="430" height="580" x="190" y="1410"/>
  <image xlink:href="data:image/png;base64, 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" width="750" height="300" x="420" y="1110"/>
  <path d="m887 2260-100-100L380 2564l100 99a720 720 0 01 407-405Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M793 2219a19 19 0 11-38 0 19 19 0 01 38 0Zm-335 336a19 19 0 11-38 0 19 19 0 01 38 0Zm116-1299 48-24 70 137-49 24Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m691 1369-48 24 32 63c10 2 22 0 31-6a40 40 0 00 17-19Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="m566 1254 18-9 74 146-17 10Zm41-21 17-8 74 146-17 9ZM960 1488h-30V1300c2-160-82-271-254-340-77-30-139-92-168-170a313 313 0 01-21-110V522h28v159c0 34 7 69 20 100a260 260 0 00 152 153C870 1008 960 1128 959 1300Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <image xlink:href="data:image/png;base64, iVBORw0KGgoAAAANSUhEUgAAAAgAAAARCAYAAADg1u3YAAAAAXNSR0IArs4c6QAAAERlWElmTU0AKgAAAAgAAYdpAAQAAAABAAAAGgAAAAAAA6ABAAMAAAABAAEAAKACAAQAAAABAAAACKADAAQAAAABAAAAEQAAAAD4dI6hAAAB+klEQVQoFWNkQAWMQK4MEOvz8fG5fPr06StIAATE2NnZTYyMjIKtra21vby8FFhZWfmcnJxWghT4pqWlpcbGxjrw8PD8O3To0MXdu3c/OXDgwNsvX748Bymo9vf353/16hXfxYsX/xgYGKj5+vqKm5iYiAPpfSAFjpaWlkXR0dHaDg4OskCFL4Am3D548OC7Y8eObQcpaO3v7+d98OCB4IYNG77//v2b387OTtLNzY07KSnpNDNQwdtLly75KCsrq5WXl1tmZ2frc3BwfFq/fv3rW7durWYBKnBPSUl5q6KiwtLX13cZaOxXJSUleX19/b9AOQ+QCQ+OHz/u8uLFCz5HR0e9zs5OexsbG5Fr1659OH/+/CKQCaZFRUWvvb29zbds2XI9MjLy+OfPn7k4OTn/AeVUgJhBlJGRcZ2WltYaoFWX9u7d+wbopp8VFRWbgHJBIBOMgZy78fHxifv377/c1dV1/eTJk9///PkjCJR7CcQMEkC8TFBQcDrQDUdnzpx5+/r16z9qa2vXA8UjQAoYpKWlF4ME161bdz4hIWGNnJzcJKDwKiCWA8mDwCyg3zuA3lubmZl5fN++fU99fHwWAsW9wbJMTExTgIKPr169+gFoxb6QkJBJvLy8PkBJZlBQg0CFmpoax927dy/8/fv3BpB/F4h/gyQAyTDOyr4NRQ0AAAAASUVORK5CYII=" width="80" height="170" x="460" y="520"/>
  <path d="m520 707 3-22H480l2 22h39Zm14-164h-65v-20h65Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <image xlink:href="data:image/png;base64, iVBORw0KGgoAAAANSUhEUgAAAAkAAAASCAYAAACJgPRIAAAAAXNSR0IArs4c6QAAAERlWElmTU0AKgAAAAgAAYdpAAQAAAABAAAAGgAAAAAAA6ABAAMAAAABAAEAAKACAAQAAAABAAAACaADAAQAAAABAAAAEgAAAAA4cj8yAAACIUlEQVQoFWNmwASsQCE1ILYF4hdA/IMRSMCAHDMzs5GJiYmfu7u7uo6ODkNYWNhpoGQBC1RFQGVlZVhgYKAXkP978+bNVxoaGp4C2UdB8kwgAghMb9269TA+Pn6pi4vLqlOnTv2MjY2VBYq7gyRhJh3//ft3Vk5Ojoazs7PMzZs3X02fPv0xUP48sqLAjIyM/6tWrTrf3Nx8WExMTMDU1JQNqEAcpAhm3am2tjZmWVlZzY0bNwZMnTrV5uvXr8JA+a9wRWxsbA7t7e3iQCvvRkdHb4iKitrByMj4momJSQykiBlE/P37V/fcuXNMIiIi8oWFhYZAq41//vz548CBAxf//PlzEKSGQU5ObibQdx9nz559DOi7pZycnJ3KysoLgOG2BCgtDAvM2by8vB+MjIz0nJycJLy8vGT////PmZ6ePu38+fMNoCBQ0dXV5VqyZEnst2/f3m7ZsuVaWlra1du3b3/98uWLHFCeA4gZlFhZWedIS0tPVVNTWwkMxGMrV668Bwyr3w4ODrOB8iIgRTqenp4zd+/e/QKo+wfQxLMBAQEbJCQkpgHlVgOxMMh3dkDvCl++fPlBVVXV0Xv37jGbm5tLVldXWzx48OAhEL8EOVxAVVW1MzEx0QjoaE2gj/4eOXLkwokTJy7u2LFj48ePH/eDHO4hKCj4hYuL6y7QpJ2HDh06CAybW0DxR0D8H4jBgAtIpgGxKhDDIhwsASMA50TRUTeZ3uAAAAAASUVORK5CYII=" width="90" height="180" x="900" y="1330"/>
  <path d="m926 1318-2 22h42l-2-22h-38Zm51 176a33 33 0 01-64 0v-12h64Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M1148 3550a621 621 0 01-603-471m1208-9a620 620 0 01-143 274m-1056-600a621 621 0 01 1183-11" fill="none" stroke="#fff" stroke-linecap="round" stroke-width="4"/>
  <path d="M128 2434h210c4 0 8 3 8 8s-4 8-8 8h-210a8 8 0 11 0-16Zm1890-2250V540c0 4-4 8-8 8a8 8 0 01-9-8V184c0-5 4-8 8-8 5 0 9 3 9 8Zm305 1751h128v70h-128Z" fill="#fff" stroke="#000" stroke-width="4"/>
  <path d="M2398 1935h53v70h-53Zm-74 652c0-2 2-4 4-5 10-10 23-13 31-23 26-30-13-50-34-67-5-5-9-11-10-18-2-12 4-24 15-29 17-9 41-20 38-42-5-34-47-26-52-55-8-40 70-40 50-83-10-21-41-24-48-46-5-15 9-27 21-33 17-7 35-16 32-39-3-13-10-24-20-32-12-10-29-13-34-27a80 80 0 01-2-31v-52h162v643h-161v-30a50 50 0 01 8-31ZM393 186v-68H460v68Zm34 0v-68" fill="#fff" stroke="#000" stroke-width="4"/>
</svg>
)=====";
