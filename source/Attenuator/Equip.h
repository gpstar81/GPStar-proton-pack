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
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="0 0 2300 3000">
  <path d="M1302 117H1430s83-4 130 56c50 60 48 120 48 120V480s20 10 20 20v393l-108 1370H254V1620l103-184v-205s2-67-84-68c-86-2-253 0-253 0V565h118v-420h625s-6-50 42-51h470s27-2 27 20ZM306 2701l106-92 98 114-106 92Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1050 750h354v1057h-355Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m1084 989 60-172H768V1440H653v286h423l8-736Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M810 1708h168v1280H811Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m339 1917 110-127L1450 2660l-110 127Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m257 2089 70-153 1205 552-70 153Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M227 2277 250 2110l1313 190-24 166Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M250 2466 227 2300l1312-190 24 167Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1162 2902 1000 2950 650 1751l160-47Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m1323 2802-140 90L510 1845l140-90Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m326 2640-70-152 1130-517 70 152Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m448 2787-110-127 940-815 110 127Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m606 2896-140-91 670-1044 140 90Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m788 2950-160-48 350-1194 160 47Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1546 2288a652 652 0 11-1304 2 652 652 0 01 1304-2Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M895 1635a651 651 0 10 622 459v-3L1220 1092c-7-24-17-48-32-69a130 130 0 00-238 74v540Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1455 2288a560 560 0 11-1122 1 560 560 0 01 1122 0Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1404 2288a510 510 0 11-1020 1 510 510 0 01 1020 0Zm-64-767a194 194 0 11-388 0 194 194 0 01 387 0Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <g fill="#fff" stroke="#000" stroke-width="3">
    <path d="M812 2067a137 137 0 11-273 0 137 137 0 01 273 0Z"/>
    <path d="M742 2067a66 66 0 11-132 0 66 66 0 01 132 0Zm511 0a137 137 0 11-273 0 137 137 0 01 273 0Z"/>
    <path d="M1183 2067a66 66 0 11-133 0 66 66 0 01 133 0Zm-330 373a128 128 0 11-257 0 128 128 0 01 257 0Z"/>
    <path d="M792 2440a67 67 0 11-134 0 67 67 0 01 134 0Zm404 0a128 128 0 11-257 0 128 128 0 01 257 0Z"/>
    <path d="M1134 2440a67 67 0 11-133 0 67 67 0 01 133 0Zm121 293a164 164 0 11-328 0 164 164 0 01 328 0Z"/>
  </g>
  <path d="M1427 454h155c12 0 22 10 22 22v398c0 12-10 23-22 23h-155c-13 0-23-10-23-23V476c0-12 10-22 23-22Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1456 501h162V560h-162Zm0 145h162v58h-162ZM1618 790h-162v60h40v60h58v-60h64v-60ZM913 923a53 53 0 11-107 0 53 53 0 01 107 0ZM380 1232h270c13 0 23 10 23 23v160c0 13-10 23-22 23H379c-12 0-22-10-22-23v-160c0-13 10-23 22-23Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M660 1334a61 61 0 11-124 0 61 61 0 01 123 0Zm-167 0a61 61 0 11-122 0 61 61 0 01 122 0ZM96 1157h171v756H96Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M90 1143h183v23H90Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M64 587h192c12 0 22 10 22 22v520c0 11-10 22-22 22H64c-12 0-22-10-22-23V610c0-12 10-22 22-22Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M30 1084h60v78H60a28 28 0 01-29-28v-50ZM30 960h60v68H30Zm0-125h60v69H30ZM30 710h60v69H30Zm28-135h30v79H30v-50c0-16 13-29 28-29Zm76 68h69v442h-69ZM1210 1116a126 126 0 11-252 0 126 126 0 01 253 0Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1152 1116a68 68 0 11-135 0 68 68 0 01 135 0ZM471 97h127v105H470Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M665 1095V131H403v992c0 53 60 96 131 96 73 0 131-43 131-96ZM1280 140h270c12 0 22 10 22 22v241c0 12-10 23-22 23h-270c-12 0-22-10-22-23V162c0-13 10-23 23-23Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M823 116h423c31 0 56 25 56 56v595c0 31-25 56-56 56H823a56 56 0 01-56-56V172c0-30 25-56 56-56Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M814 100h65v66h-65Zm376 0h65v66h-64Zm-250 0h64v66H940Zm125 0h65v66h-65Zm192 86h58v67h-58Zm0 94h58v65h-58Zm0 378h58v66h-58ZM755 186h58v67h-58Zm0 94h58v65h-58Zm0 378h58v66h-58Zm478 120h81V790c0 25-20 45-45 45h-36v-58Zm-478 0h81v58H800c-25 0-45-20-45-45v-14Zm382 0h82v58h-82Zm-95 0h81v58h-81Zm-96 0h81v58h-80Zm-96 0h82v58H850Zm-2-449H1220v223H848Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M848 396H1220v156H848Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M849 550V394c-2-35-82-32-82-32v220s80-2 82-32Zm453 55v-260c0 40-32 38-32 38h-50v183h45s38-3 37 39Zm-290-298a66 66 0 01 48-123c63 23 77 16 78 18 5 4-14-5-40 68a66 66 0 01-86 37Zm127-105L976 270" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1146 210a30 30 0 11-60 0 30 30 0 01 60 0ZM883 1541a79 79 0 11-158 0 79 79 0 01 158 0ZM640 1597a79 79 0 11-157 0 79 79 0 01 158 0Zm886-1340h-30l-14 26 14 25h30l14-25-14-26Zm-134 0H1360l-17 28 17 28h32l17-28-17-28ZM790 1219h43v60H790Zm0 102h43v59H790Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M673 1160h130v297h-130Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M803 1162c0 47-35 84-65 84s-64-37-64-84c0-46 34-83 64-83 30 0 65 37 65 83Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M360 502h348v644H361Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M435 648h198v35H435Zm0 131h198v35H435Zm0 129h198v35H435Zm2 238v-79h51l24-47h44l25 47h51v80H437Zm-2-644h198v55H435Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M416 520h30v583h-30Zm201-1h30v584h-30Zm421 1015a20 20 0 01 0-27l48-47a86 86 0 01 120 0l48 47c7 7 7 20 0 27l-47 47a86 86 0 01-121 0Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1220 1520a73 73 0 11-147 0 73 73 0 01 146 0Zm-442 35 25 15 26-14v-30l-25-15-26 15v30Zm-480 1132 38-33 10 12-38 33ZM137 146h225v298H137Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M260 80h32v86H260Zm58 342h31v579h-31Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M248 146h114v298H248Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M325 215a20 20 0 11-40 0 20 20 0 01 40 0Zm0 160a20 20 0 11-40 0 20 20 0 01 40 0ZM52 260H120v77H50Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M112 255h25v87h-25Zm306 153h-55V185h55Zm1199 86h14v215h-14Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1630 506h15v190H1630Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1644 480h14v240h-14ZM1350 2776l92-106 15 13-92 106Zm30-3 64-74 50 44-63 74Zm612-590v491H1900v-490Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m1992 2674-7 17h-73l-11-17h90Zm16-108v54h-123v-54Zm-33-1861v110h-129v-110Zm33 1356v156h-123V2060Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M2015 1400v747h-137V1400ZM2200 796V900h-425V796Zm-160 1821v-40h-32v100s46-58 93 2c0 0-3-60-60-62ZM2144 1390v165h-117v-166ZM1990 205h170v539h-170Zm-154 720v74H1800v-74Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1856 910v104h-24V910Zm1 115h-52v10h30v17h-30v10h30v17h-30v10h30v17h-30v10h30v17h-30v10h30v17h-30v10h30v17h-30v10h30v17h-30v10h30v17h-30v10h52v-226Zm340 135v80h-25v-80Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M2176 1122V1250h-35v-130Zm-4-74v-80h25v80Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M2141 1087V958h35v129Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M2154 900v503H1850V901Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M2140 1256v69h-55v-70Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M2145 1243v26H2080v-26Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1850 1052v200h304V973a76 76 0 01-77 78Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M2006 970V173h139V970a70 70 0 11-140 0Zm271-208v242h-99V762ZM1880 2069v77h-153V2070Zm217 11v56h-96V2080Zm-205-796h145v192h-145Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1892 1448h145v44h-145ZM1850 1250h228v39H1850Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M1878 908h44v120h-44Z" fill="none" stroke="#fff" stroke-width="3"/>
  <path d="M1886 916h28V1020h-28Zm94-238v38h-140v-38Zm172 101h18V870h-18Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m1081 2099 69-69m-32 75 38-38m-81-5 38-38M640 2099l68-69m-32 75 38-38m-81-5 38-38m361 451 68-69m-32 75 39-38m-81-5 38-38M690 2475l69-69m-32 75 38-38m-81-5 38-38" fill="none" stroke="#fff" stroke-linecap="round" stroke-linejoin="round" stroke-width="3"/>
  <path d="M1610 2316v10c0 9 5 13 13 15 5 0 16 0 16 9a8 8 0 01-9 8h-492s-35-1-52 5a61 61 0 00-32 43c0 8-4 16-8 24-5 7-13 11-21 11H767c-8 0-16-4-21-11-5-8-7-16-8-24a60 60 0 00-33-43c-16-6-52-5-52-5h-490a8 8 0 01-10-8c0-8 11-8 17-10 7 0 13-6 13-14v-105c0-8-6-12-13-14-6-1-16-1-17-10a8 8 0 01 10-8h490s37 2 53-5a60 60 0 00 32-43c1-8 4-16 8-23 5-7 13-12 22-12h257c9 0 17 5 22 12 4 7 7 15 8 23 4 18 15 36 32 43 16 7 52 5 52 5H1630a8 8 0 01 9 9c0 8-10 8-16 9-8 2-13 6-13 14v95Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M976 2272a85 85 0 11-170 0 85 85 0 01 170 0Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M920 2272a28 28 0 11-57 0 28 28 0 01 56 0ZM859 876c-291 5-562 151-646 443-65 222-5 440 128 664L590 1738c-115-120-248-295-210-475C425 1050 667 970 860 969a46 46 0 00 0-93" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M604 1737A815 815 0 01 380 1311a447 447 0 00-227 261c33 160 93 306 194 427Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <image xlink:href="data:image/png;base64, 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" width="450" height="680" x="160" y="1310"/>
  <path d="M486 1102a410 410 0 00-179 18c-167 170-154 450-154 450v1a660 660 0 01 88-134 468 468 0 01 172-133 243 243 0 01 73-203Z" fill="#fff" stroke="#000" stroke-linejoin="round" stroke-width="3"/>
  <image xlink:href="data:image/png;base64, iVBORw0KGgoAAAANSUhEUgAAACMAAAAuCAYAAACiR6CHAAAAAXNSR0IArs4c6QAAAERlWElmTU0AKgAAAAgAAYdpAAQAAAABAAAAGgAAAAAAA6ABAAMAAAABAAEAAKACAAQAAAABAAAAI6ADAAQAAAABAAAALgAAAABALIcWAAAFNElEQVRYCcXW2XNUVRTFYREFB5SgDEks0YDKYCFWWWis0vLNP1sffJJBUBBlDooggYAjojitr+vuVCfcDt2ddGdX/erc8Zx11t77dm94bHSxKVNPNGxrxqczPhE2hv/C3835rxmPubHa2JAJnmp4LuOOMBUc32+w6F/hQSDyyWZ8POMf4UK4a6LVhJ2/EjaHZ4NzO/4p/Bu2BtcdW5QYQYTnjNfC2fCAXcOGjUwHLsBc7BYvBgJ/D78FIqRIumSDS/8Ezp1qzjsT5HjgIORgOBT+DLcCFwhYCERJE1fct3DhOfek6mj4JXRi2DTtzNuzQTpqYpNa4PlAgF1zw31O3AuuSxcXvdtJT8ZODCNmKm/uDSbcEqSA9VypupAOdTIfpIkwYgouec8zizFMN9nARNBBN8OusBDsdDLonNNBYVrQwhZVsDqJUNduhyUxqDMzefutYHEpMbFj4kx+vhlr4eoyhb47qBf1dCx8H5bEIM5ICzE6RB0IznBEIUqRVn0hSKXiJVZnqRn1ImXW9N5DMYiYj/I2u6XHxFeCmiCOEz52+4P7CveZ4B4nFLWU4WIg7qHoV4wda999QT2YUN6leWc4EjhxJxBl54QTwQ2ucVMBXw+t0U/NKLoDgf0ng7QQx/49QdFygBueJdJIRF37Ocfc0FVS2upMP2IUYRWoheyWQ3bqul1zxa49K2WekyZFzVEj16TX/dboR0z3i6/l5PVg0qlwN3BBXWwPHCJSp0glFz1L5ExQT9xtjX7EeMZkh8NLwYImNXJGUUobZ+z6RFgIWp+wN4JNcPBqOB5a41EFTMiuYEI1QoBFpMFCdq1Q1ce18F0w50xQS8QLQtSPD2LPeJQYIkzMCQtuCRxwXkV4Nsc3mvPpjNLIya2Ba96xiaqlHLbHSmLezSt2xwVCHP8QLOKa+CTYtTS9FzjBTfcJIYobrjk39oxeYrzEfhMZdcZ8UCN2eT2cCZvCTDgUai6LSiGR3HCu22yoZyfl3uIEjrtjR05M6OUS5lybXgg3A4FEcMxz3PEsN3SR9DgnslL5Y457Ru1m+QN2Y7L6oprURF8EO94ePgzSwTk7vx8IKSdz2Hnny4zEVWpdbw2LLA/dciRYgEO3w0Ko78PuHL8T2K5+uEKgj6FNmBNHw1xYMTW5vxhtzqgDu9sXzgcpYbM0HAivBp1EiJqymLbljF/2y+GrUN2Ww/6iTYwvq4VuhCsNhMwGu7drgi2uxct+oo4HX98Vvye53xptYkxkkVPBLqXt/UCQOtJNhBAmPZ71Q/hpcJ2ooWK5GAU4GXQMVwh5O0gDtwhRSwQ7dt2zJ0M5lMPhQo67Q/u6djFIx8eBG65ZTBfZvZriyLfh66BwVx0WbAuOzYaXQ328CFIr3FOc2lwa10RI5unUgbE71MbBYOeHAyeIVgsEQdueC2smJHN1FjF2x56caGEuECY9hDgmUJd9FlZdI5ljSSwv4OncJUQQoFCNnJGeb4JvzpoLyZxL0qR4PwjSoovUhd8f/1kImQunw8Afs7zTV8h/xZs54IIWJoA4fye18nz4PIzEkczbCXVQcSsHBNg5ZwjS1neCz7uUjTRKjN1PBU5VGtQJfEf86o48SoyvqZQYJ4KPmm7ykyBFY4kSIx2Twefd74w0aeFzYWxRYhSm9KgTgjh0Jow1SozC1cbbgnTNhbHUSdZZjBJzL1d82tWHa5eCNh9rlBiuSI//KCP/nvTaYYlRL9rYn2dpWpcoMdqYiKvroqJZlBgo4Lkw8q9s1lgxpIcg47rG/7caXqnFP5nVAAAAAElFTkSuQmCC" width="350" height="460" x="140" y="1100"/>
  <image xlink:href="data:image/png;base64, iVBORw0KGgoAAAANSUhEUgAAADwAAAAZCAYAAABtnU33AAAAAXNSR0IArs4c6QAAAERlWElmTU0AKgAAAAgAAYdpAAQAAAABAAAAGgAAAAAAA6ABAAMAAAABAAEAAKACAAQAAAABAAAAPKADAAQAAAABAAAAGQAAAACC/kkoAAAEA0lEQVRYCd3WWU9UQRQEYBX3fYuKICJE0fj/f4aJMTzwoAKSGBSVRVxQEeubmWOuExLJOAyDJyl6ud13TlWd7svIkcMdx7rSP5HxSGfuVNofXc9/P+yeH7bx8SRUOJ/+z2AiQO5mZ7yV9mwwFSA7GojP7ab992hzMET908nlZPAtuBOsBTPBh8A8QtudvlZsBPOBvfeCC8H34GMwF7TcLvszPrBolqV8HgWSfhxwiZvTwdcA0YsB184FnokzwU5wN1gJljrjK2mZynWibR0UYSQ5oEzHGn1ubQYS1YdLAZcEd78EBOGe595V5awixgPCvAveBlcDYnB44yBK2tlDVNIEvxzIA1EEJP06QIKbnl8PEOOyfUhztQin2yJUx8CzTwFXFwPn3LOlQRK+nx+8EXALURcM95xLyV0LlOj7gMOIPw8kjeRkoMSdVfsI5D04EELFmCOQPUremGhPAr+xvZ+EuUhpJDgBxtxCjuKS55yyk5DEkVWuXJS887oYrAfeOR0oWWS9wx4CeE5EgiJbVWDezT4brO4HYQSmAskgxylJK1HniOqSIgBySPl+SpIgCDhzxtU3vxrYtxIIZco9JIllDaH8rt8iDrFVhz0LwWY/Cfsht6RWeSEHBDAWyBKAQ8LFAuU+pxBBFgkhYS4RxxwS3HsZcPl2MB4Qw3v9hrXEIoB2ITC/0w/C3jERICUZypqTjB/mHoUlKSHnWCLOmXXWIPwqWAi8QwnOBNx309qzHlhPgDfBWPA0cAdYPxkQz/yLgHgqqyoi3fYLWp0e/3BCMlpAFlGuCqpTWLkpPYQpjbC+OUKUGOaWA+WLFCKqxnruEs8zAlXZIrXnoFiv8SAbuUpJBDmjXAvpttwghDNpXpJ1Pq2XrBysIYQjIJCbDxArV4nyz9EL4dH8Kle5JhBABCHv4wIhkOAMUhJHRjm6aJShPdbZg5z3KEmttQTpe/RCGFFklK79WuOCslTGIHlhDaKIIGufcp0NfJoGFnsl7GJBSHTv4R4nkdEaI+g8c5RAyCFvLxetcRkNPLqT707A2eTMrcCNKco1rbKTvL7W+4qsMfdcMuBzBQN1NL/3R/yNsPJ8GFjnjIrmHn03LRe1XHU2fUJcPPoEmAuGIprJNxPi7FgwEShLSSvX+uzYVw5zWTkj5ybVtx9p5Yv40MRuhN2y44HPjahziWARNa9fqDImzkrgE7McDF3sRpg7EveMe4CsqLY9aq/RtwdprduZSEMZTcLKFeqzg7Q+6HO+uT7D1kWFoLJ9ZmLYA0HBGWd2MjDHIc5WObtZ6xZOtxVV4p45r4ciOKYUnVmfHReP29jFg2C1iHeXszGhrO/Lv315z74HwuD7KHkkoXluM/x/4hfJqRq9W+AGOwAAAABJRU5ErkJggg==" width="600" height="250" x="320" y="860"/>
  <path d="m691 1765-78-78-317 317 77 77a562 562 0 01 318-316Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M618 1733a15 15 0 11-30 0 15 15 0 01 30 0Zm-261 263a15 15 0 11-30 0 15 15 0 01 30 0Zm90-1015 38-19 54 107-38 20Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="m539 1070-38 18 25 50c9 0 17 0 25-5 5-3 10-9 13-15Z" fill="#fff" stroke="#000" stroke-width="3"/>
  <path d="M895 2773a485 485 0 01-470-368m941-7c-18 80-56 153-110 214m-824-468a485 485 0 01 922-9" fill="none" stroke="#fff" stroke-linecap="round" stroke-width="3"/>
  <path d="M100 1901h163c4 0 7 3 7 7 0 4-3 6-7 6H100a6 6 0 01 0-13ZM1573 144v278c0 3-3 6-6 6a6 6 0 01-7-6V144c0-4 3-7 7-7 3 0 6 3 6 7Zm238 1368h100v55h-100Zm1 508 3-3c7-7 18-10 24-18 20-24-10-40-26-52-4-4-7-9-8-14-1-10 3-19 12-23 13-7 32-15 30-33-5-26-37-20-42-42-6-32 55-31 40-65-8-17-32-19-38-36-3-12 7-22 17-26 13-6 27-13 24-30a40 40 0 00-15-26c-10-7-23-10-27-20-2-9-2-17-1-25v-40h126v502h-126v-24c0-8 2-17 7-24Z" fill="#fff" stroke="#000" stroke-width="3"/>
</svg>
)=====";
