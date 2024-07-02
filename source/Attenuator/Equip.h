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
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1150 1500">
  <path d="M651 59h64s40-2 65 27c24 30 24 60 24 60V240s10 5 10 10v197l-55 685H127V810l51-92V616s1-34-42-35H10V282h60v-210h310s-3-24 22-25h235s13-1 13 10ZM153 1350l53-45 49 56-53 46Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M525 375h177v528H525Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m542 494 30-85H384v310h-57v143h211l4-368Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M405 854H490v640h-84Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m170 958 54-63L725 1330l-55 63Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m128 1044 35-76 603 276-35 76Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m113 1138 12-83 656 95-12 83Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m125 1233-12-83 656-95 12 83Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m580 1451-80 24-175-600 80-23Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m662 1400-70 46-337-524 70-45Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m163 1320-35-76 565-259 35 77Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M224 1393 170 1330l470-408 55 64Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m303 1448-70-46L568 880l70 46Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m394 1475-80-24 175-597 80 24Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M773 1144a326 326 0 11-652 0 326 326 0 01 652 0Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M447 818a326 326 0 10 311 229v-1l-147-500c-4-12-10-24-16-34a65 65 0 00-120 37v270Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M727 1144a280 280 0 11-560 0 280 280 0 01 560 0Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M702 1144a255 255 0 11-510 0 255 255 0 01 510 0ZM670 760a97 97 0 11-194 0A97 97 0 01 670 760Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <g fill="#fff" stroke="#000" stroke-width="2">
    <path d="M406 1033a68 68 0 11-136 0 68 68 0 01 136 0Z"/>
    <path d="M371 1033a33 33 0 11-66 0 33 33 0 01 66 0Zm255 0a68 68 0 11-136 0 68 68 0 01 136 0Z"/>
    <path d="M591 1033a33 33 0 11-66 0 33 33 0 01 66 0ZM426 1220a64 64 0 11-128 0 64 64 0 01 128 0Z"/>
    <path d="M396 1220a33 33 0 11-67 0 33 33 0 01 67 0Zm202 0a64 64 0 11-128 0 64 64 0 01 128 0Z"/>
    <path d="M567 1220a33 33 0 11-67 0 33 33 0 01 67 0Zm60 147a82 82 0 11-163 0 82 82 0 01 163 0Z"/>
  </g>
  <path d="M713 227h78c6 0 11 5 11 11v200c0 5-5 10-11 10h-78a10 10 0 01-10-10v-200c0-6 4-11 10-11Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M728 250h80v30h-80Zm0 73h80v30h-80Zm81 72h-81v30h20v30h29v-30h32v-30Zm-353 66a27 27 0 11-53 0 27 27 0 01 53 0ZM190 616h135c7 0 12 5 12 11v80c0 7-5 12-12 12H190c-7 0-12-5-12-11v-80c0-7 5-12 12-12Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M330 667a30 30 0 11-62 0 30 30 0 01 62 0Zm-83 0a30 30 0 11-62 0 30 30 0 01 62 0ZM48 578h86v378H48Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M45 571h92v12H45Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M32 293h96c6 0 11 5 11 12v260c0 5-5 10-11 10H32c-6 0-11-5-11-10v-260c0-7 5-12 11-12Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M15 542h30v40H28c-8 0-14-7-14-15v-25Zm0-62h30v34h-30Zm0-63h30v35h-30Zm0-62h30v34h-30ZM30 288h14v39H15v-25c0-8 6-14 14-14Zm37 34h34v220H67Zm538 236a63 63 0 11-126 0 63 63 0 01 126 0Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M576 558a34 34 0 11-68 0 34 34 0 01 68 0Zm-340-510h63v53h-63Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M333 548V65H202v496c0 27 30 49 65 49 36 0 66-22 66-49ZM640 70h135c6 0 11 5 11 10v122c0 6-5 10-11 10H640c-6 0-10-4-10-10V80c0-6 4-11 10-11Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M411 58h212c16 0 28 13 28 28v298c0 15-12 28-28 28H411a28 28 0 01-28-28V86c0-15 13-28 28-28Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M407 50h32v33h-32Zm188 0h33v33h-33ZM470 50h32v33H470Zm63 0h32v33h-32Zm95 43h30v33h-30Zm0 47h30v33h-30Zm0 190h30v32h-30ZM377 93h30v33h-30Zm0 47h30v33h-30Zm0 190h30v32h-30Zm239 59h41v7c0 12-10 22-22 22h-19v-30Zm-239 0h41v29H400c-13 0-23-10-23-23V390Zm192 0h40v30h-40Zm-49 0h42v30H520Zm-47 0h40v30h-40Zm-48 0h40v30h-40Zm-1-225H610v112H424Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M424 198H610v78H424Zm0 77v-78c0-17-40-16-40-16v110s40 0 40-16Zm227 27v-130c0 20-16 20-16 20H610v91h23s19-2 18 20ZM506 154A33 33 0 01 530 92c31 12 38 8 40 9 2 2-8-3-21 34a33 33 0 01-43 19ZM570 100l-82 35" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M573 105a15 15 0 11-30 0 15 15 0 01 30 0ZM441 770a40 40 0 11-78 0 40 40 0 01 78 0ZM320 798a40 40 0 11-79 0 40 40 0 01 80 0Zm443-669h-15l-7 12 7 13h15l7-13-7-12Zm-67-1H680l-8 14 8 15h16l8-15-8-14ZM395 610h22v30h-22Zm0 50h22v30h-22Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M337 580H400v148h-64Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M402 581c0 23-18 42-33 42-15 0-32-19-32-42 0-23 17-41 32-41 15 0 33 18 33 41Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M180 251h174v322H180Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M218 324h99V340h-100Zm0 66h99v17h-100Zm0 64h99v17h-100Zm0 119v-40h26l12-23h22l12 24h26v40h-98Zm0-322h99v27h-100Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M208 260h15v292h-15Zm100 0h16v290h-16ZM520 767a10 10 0 01 0-13l23-24a43 43 0 01 60 0l24 24c4 3 4 10 0 13L603 790a43 43 0 01-60 0Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M610 760a37 37 0 11-73 0 37 37 0 01 73 0Zm-221 18 13 7 12-7v-15l-12-7-13 7v15Zm-240 566 19-17 5 6-19 17ZM69 73H180v150H69Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M130 40h16v43H130Zm29 171h16v290h-16Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M124 73h57v150h-57Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M162 108a10 10 0 11-20 0 10 10 0 01 20 0Zm0 79a10 10 0 11-20 0 10 10 0 01 20 0ZM26 130H60v38H26Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M56 127h12v44H56ZM210 204h-30V93h30Zm599 43h6v108H810Zm6 6h7v95h-7Zm7-13h7v120h-7ZM675 1388l46-53 8 6-46 54Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m690 1386 32-36 25 22-31 37Zm306-294v245H950v-245Zm0 245-3 9h-37l-6-9h46Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M1004 1283v27h-62v-27Zm-16-930v54h-65v-54Zm16 677v78h-62v-77Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M1008 700v373h-70V700Zm92-302V450H888v-52Zm-80 911v-20h-16v50s23-30 46 0c0 0 0-30-30-30Zm52-614v82h-58v-82Zm-77-593h85v270h-85Zm-77 361v36H900v-36Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M928 455v52h-12v-52Zm0 57h-25v5h15v9h-15v5h15v8h-15v5h15v9h-15v5h15v8h-15v5h15v9h-15v5h15v8h-15v5h15v9h-15v5h15v8h-15v5h25V512Zm170 68v40h-12v-40Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M1088 560v65h-17v-64Zm-2-36v-40h12v40Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M1070 543V480h18v64Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M1077 450v252H925V450Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M1070 628v35h-28v-35Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M1073 621v13H1040v-13Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M925 526v100h152v-140a38 38 0 01-39 40Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M1003 485V86h70v400a35 35 0 11-70 0Zm136-104v120h-50v-120ZM940 1034v40h-76v-40Zm109 6v28H1000V1040ZM946 642h72v96h-72Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M946 724h72v22h-72Zm-21-99H1040v20H925Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M939 454H960v60h-20Z" fill="none" stroke="#fff" stroke-width="2"/>
  <path d="M943 458h14V510h-14ZM990 340v18h-70V340Zm86 50h10v45h-10Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m540 1050 35-35m-16 38 19-20m-41-2 20-20M320 1050l34-35m-16 38 20-20m-41-2 19-20m180 226 34-34m-16 37 20-19m-41-1 19-20m-187 37 34-34m-16 37 20-19m-41-1 20-20" fill="none" stroke="#fff" stroke-linecap="round" stroke-linejoin="round" stroke-width="2"/>
  <path d="M805 1158v5c0 4 2 6 6 7 3 0 8 0 8 5a4 4 0 01-4 4H570s-18 0-26 3a30 30 0 00-16 21c0 4-2 8-4 12-2 3-6 6-10 6h-130c-4 0-8-3-10-6-2-4-4-8-4-12a30 30 0 00-16-21c-8-4-26-3-26-3H80a4 4 0 01-4-4c0-4 5-4 8-5 4 0 6-3 6-7v-52c0-5-2-7-6-8-3 0-8 0-8-4a4 4 0 01 4-4h246s18 0 26-3a30 30 0 00 16-21c0-4 2-8 4-12 3-4 7-6 10-6h130c4 0 8 2 10 6 2 4 4 8 4 12 3 8 8 17 17 21 8 3 26 3 26 3h245a4 4 0 01 4 4c0 4-5 4-8 4-4 1-6 3-6 8v47Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M488 1136a42 42 0 11-85 0 42 42 0 01 85 0Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M460 1136a14 14 0 11-28 0 14 14 0 01 28 0Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <g fill="#fff" stroke="#000" stroke-width="2">
    <path d="M430 438c-146 3-281 76-324 221-32 111-2 220 65 332L294 870C237 810 170 722 190 631c22-106 143-146 240-146a23 23 0 00 0-47"/>
    <path d="M302 869A408 408 0 01 190 655 224 224 0 00 77 786a470 470 0 00 97 213Z"/>
    <path d="M243 551c-30-4-60 0-90 9C70 646 77 786 77 786 89 762 103 740 120 720c27-32 56-54 87-67a121 121 0 01 36-101Z" stroke-linejoin="round"/>
  </g>
  <path d="m346 883-40-40-158 159 39 39c29-73 86-130 159-158Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M310 867a7 7 0 11-16 0 7 7 0 01 15 0ZM179 998a7 7 0 11-15 0 7 7 0 01 15 0ZM224 490l18-9 27 54-18 10Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="m270 535-20 10 13 24c4 0 9 0 12-2l7-8Z" fill="#fff" stroke="#000" stroke-width="2"/>
  <path d="M447 1387a242 242 0 01-235-184m471-3a250 250 0 01-55 106m-412-234a242 242 0 01 461-5" fill="none" stroke="#fff" stroke-linecap="round" stroke-width="2"/>
  <path d="M50 950h82c1 0 3 2 3 4s-2 3-3 3H50a3 3 0 01 0-6ZM787 72V210c0 2-2 3-3 3a3 3 0 01-4-3V70l3-2c2 0 4 1 4 3Zm119 684h50v27h-50Zm0 254 1-1c4-4 10-6 13-10 10-11-5-20-13-26-3-1-4-4-4-7-1-4 1-9 5-10 7-4 16-9 15-17-2-14-18-10-20-22-3-15 27-15 20-32-4-9-17-10-20-18-1-6 4-10 9-13 6-3 13-6 12-15 0-5-4-10-8-13-4-3-10-5-13-10v-33h62v251h-62v-11c0-5 0-9 3-13Z" fill="#fff" stroke="#000" stroke-width="2"/>
</svg>
)=====";
