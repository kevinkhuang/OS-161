
/* The goal here is to produce lots of .text pages.
 * We'd like to create a lot of page faults on text pages.
 */

/* keep this small */
#define FUNCTION_LOOP (2)

/* Returning int and using volatile to try to prevent
 * optimizer from eliminating too much code
 */

#define FUNC(num) \
int func_##num() \
{ \
	int i; \
  volatile int l = 10; \
  volatile int m = 11; \
  volatile int n = 12; \
 \
  for (i=0; i<FUNCTION_LOOP; i++) { \
    l = l + m - n + i; \
    m = l + m - n + i; \
    n = l + m - n + i; \
    l = l + m - n + i; \
    m = l + m - n + i; \
    n = l + m - n + i; \
    l = l + m - n + i; \
    m = l + m - n + i; \
    n = l + m - n + i; \
    l = l + m - n + i; \
    m = l + m - n + i; \
    n = l + m - n + i; \
    l = l + m - n + i; \
    m = l + m - n + i; \
    n = l + m - n + i; \
    l = l + m - n + i; \
  } \
  for (i=0; i<FUNCTION_LOOP; i++) { \
    m = l + m - n + i; \
    l = l + m - n + i; \
    n = l + m - n + i; \
    m = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    n = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    n = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    n = l + m - n + i; \
  } \
  for (i=0; i<FUNCTION_LOOP; i++) { \
    n = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    n = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    n = l + m - n + i; \
  } \
  for (i=0; i<FUNCTION_LOOP; i++) { \
    l = l + m - n + i; \
    n = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    n = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    n = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    n = l + m - n + i; \
    m = l + m - n + i; \
    l = l + m - n + i; \
    m = l + m - n + i; \
  } \
 \
 \
  return l + m + n; \
}


/* trying to create a process with a fair sized text segment */
FUNC(0); FUNC(1); FUNC(2); FUNC(3); FUNC(4);
FUNC(5); FUNC(6); FUNC(7); FUNC(8); FUNC(9);
FUNC(10); FUNC(11); FUNC(12); FUNC(13); FUNC(14);
FUNC(15); FUNC(16); FUNC(17); FUNC(18); FUNC(19);
FUNC(20); FUNC(21); FUNC(22); FUNC(23); FUNC(24);
FUNC(25); FUNC(26); FUNC(27); FUNC(28); FUNC(29);
FUNC(30); FUNC(31); FUNC(32); FUNC(33); FUNC(34);
FUNC(35); FUNC(36); FUNC(37); FUNC(38); FUNC(39);
FUNC(40); FUNC(41); FUNC(42); FUNC(43); FUNC(44);
FUNC(45); FUNC(46); FUNC(47); FUNC(48); FUNC(49);
FUNC(50); FUNC(51); FUNC(52); FUNC(53); FUNC(54);
FUNC(55); FUNC(56); FUNC(57); FUNC(58); FUNC(59);
FUNC(60); FUNC(61); FUNC(62); FUNC(63); FUNC(64);
FUNC(65); FUNC(66); FUNC(67); FUNC(68); FUNC(69);
FUNC(70); FUNC(71); FUNC(72); FUNC(73); FUNC(74);
FUNC(75); FUNC(76); FUNC(77); FUNC(78); FUNC(79);
FUNC(80); FUNC(81); FUNC(82); FUNC(83); FUNC(84);
FUNC(85); FUNC(86); FUNC(87); FUNC(88); FUNC(89);
FUNC(90); FUNC(91); FUNC(92); FUNC(93); FUNC(94);
FUNC(95); FUNC(96); FUNC(97); FUNC(98); FUNC(99);
FUNC(100); FUNC(101); FUNC(102); FUNC(103); FUNC(104);
FUNC(105); FUNC(106); FUNC(107); FUNC(108); FUNC(109);
FUNC(110); FUNC(111); FUNC(112); FUNC(113); FUNC(114);
FUNC(115); FUNC(116); FUNC(117); FUNC(118); FUNC(119);
FUNC(120); FUNC(121); FUNC(122); FUNC(123); FUNC(124);
FUNC(125); FUNC(126); FUNC(127); FUNC(128); FUNC(129);
FUNC(130); FUNC(131); FUNC(132); FUNC(133); FUNC(134);
FUNC(135); FUNC(136); FUNC(137); FUNC(138); FUNC(139);
FUNC(140); FUNC(141); FUNC(142); FUNC(143); FUNC(144);
FUNC(145); FUNC(146); FUNC(147); FUNC(148); FUNC(149);
FUNC(150); FUNC(151); FUNC(152); FUNC(153); FUNC(154);
FUNC(155); FUNC(156); FUNC(157); FUNC(158); FUNC(159);
FUNC(160); FUNC(161); FUNC(162); FUNC(163); FUNC(164);
FUNC(165); FUNC(166); FUNC(167); FUNC(168); FUNC(169);
FUNC(170); FUNC(171); FUNC(172); FUNC(173); FUNC(174);
FUNC(175); FUNC(176); FUNC(177); FUNC(178); FUNC(179);
FUNC(180); FUNC(181); FUNC(182); FUNC(183); FUNC(184);
FUNC(185); FUNC(186); FUNC(187); FUNC(188); FUNC(189);
FUNC(190); FUNC(191); FUNC(192); FUNC(193); FUNC(194);
FUNC(195); FUNC(196); FUNC(197); FUNC(198); FUNC(199);
FUNC(200); FUNC(201); FUNC(202); FUNC(203); FUNC(204);
FUNC(205); FUNC(206); FUNC(207); FUNC(208); FUNC(209);
FUNC(210); FUNC(211); FUNC(212); FUNC(213); FUNC(214);
FUNC(215); FUNC(216); FUNC(217); FUNC(218); FUNC(219);
FUNC(220); FUNC(221); FUNC(222); FUNC(223); FUNC(224);
FUNC(225); FUNC(226); FUNC(227); FUNC(228); FUNC(229);
FUNC(230); FUNC(231); FUNC(232); FUNC(233); FUNC(234);
FUNC(235); FUNC(236); FUNC(237); FUNC(238); FUNC(239);
FUNC(240); FUNC(241); FUNC(242); FUNC(243); FUNC(244);
FUNC(245); FUNC(246); FUNC(247); FUNC(248); FUNC(249);
FUNC(250); FUNC(251); FUNC(252); FUNC(253); FUNC(254);
FUNC(255); FUNC(256); FUNC(257); FUNC(258); FUNC(259);
FUNC(260); FUNC(261); FUNC(262); FUNC(263); FUNC(264);
FUNC(265); FUNC(266); FUNC(267); FUNC(268); FUNC(269);
FUNC(270); FUNC(271); FUNC(272); FUNC(273); FUNC(274);
FUNC(275); FUNC(276); FUNC(277); FUNC(278); FUNC(279);
FUNC(280); FUNC(281); FUNC(282); FUNC(283); FUNC(284);
FUNC(285); FUNC(286); FUNC(287); FUNC(288); FUNC(289);
FUNC(290); FUNC(291); FUNC(292); FUNC(293); FUNC(294);
FUNC(295); FUNC(296); FUNC(297); FUNC(298); FUNC(299);

#define CALLFUNC(i) \
	func_##i() 

void
call_all()
{
	CALLFUNC(0); CALLFUNC(1); CALLFUNC(2); CALLFUNC(3); CALLFUNC(4);
	CALLFUNC(5); CALLFUNC(6); CALLFUNC(7); CALLFUNC(8); CALLFUNC(9);
	CALLFUNC(10); CALLFUNC(11); CALLFUNC(12); CALLFUNC(13); CALLFUNC(14);
	CALLFUNC(15); CALLFUNC(16); CALLFUNC(17); CALLFUNC(18); CALLFUNC(19);
	CALLFUNC(20); CALLFUNC(21); CALLFUNC(22); CALLFUNC(23); CALLFUNC(24);
	CALLFUNC(25); CALLFUNC(26); CALLFUNC(27); CALLFUNC(28); CALLFUNC(29);
	CALLFUNC(30); CALLFUNC(31); CALLFUNC(32); CALLFUNC(33); CALLFUNC(34);
	CALLFUNC(35); CALLFUNC(36); CALLFUNC(37); CALLFUNC(38); CALLFUNC(39);
	CALLFUNC(40); CALLFUNC(41); CALLFUNC(42); CALLFUNC(43); CALLFUNC(44);
	CALLFUNC(45); CALLFUNC(46); CALLFUNC(47); CALLFUNC(48); CALLFUNC(49);
	CALLFUNC(50); CALLFUNC(51); CALLFUNC(52); CALLFUNC(53); CALLFUNC(54);
	CALLFUNC(55); CALLFUNC(56); CALLFUNC(57); CALLFUNC(58); CALLFUNC(59);
	CALLFUNC(60); CALLFUNC(61); CALLFUNC(62); CALLFUNC(63); CALLFUNC(64);
	CALLFUNC(65); CALLFUNC(66); CALLFUNC(67); CALLFUNC(68); CALLFUNC(69);
	CALLFUNC(70); CALLFUNC(71); CALLFUNC(72); CALLFUNC(73); CALLFUNC(74);
	CALLFUNC(75); CALLFUNC(76); CALLFUNC(77); CALLFUNC(78); CALLFUNC(79);
	CALLFUNC(80); CALLFUNC(81); CALLFUNC(82); CALLFUNC(83); CALLFUNC(84);
	CALLFUNC(85); CALLFUNC(86); CALLFUNC(87); CALLFUNC(88); CALLFUNC(89);
	CALLFUNC(90); CALLFUNC(91); CALLFUNC(92); CALLFUNC(93); CALLFUNC(94);
	CALLFUNC(95); CALLFUNC(96); CALLFUNC(97); CALLFUNC(98); CALLFUNC(99);
	CALLFUNC(100); CALLFUNC(101); CALLFUNC(102); CALLFUNC(103); CALLFUNC(104);
	CALLFUNC(105); CALLFUNC(106); CALLFUNC(107); CALLFUNC(108); CALLFUNC(109);
	CALLFUNC(110); CALLFUNC(111); CALLFUNC(112); CALLFUNC(113); CALLFUNC(114);
	CALLFUNC(115); CALLFUNC(116); CALLFUNC(117); CALLFUNC(118); CALLFUNC(119);
	CALLFUNC(120); CALLFUNC(121); CALLFUNC(122); CALLFUNC(123); CALLFUNC(124);
	CALLFUNC(125); CALLFUNC(126); CALLFUNC(127); CALLFUNC(128); CALLFUNC(129);
	CALLFUNC(130); CALLFUNC(131); CALLFUNC(132); CALLFUNC(133); CALLFUNC(134);
	CALLFUNC(135); CALLFUNC(136); CALLFUNC(137); CALLFUNC(138); CALLFUNC(139);
	CALLFUNC(140); CALLFUNC(141); CALLFUNC(142); CALLFUNC(143); CALLFUNC(144);
	CALLFUNC(145); CALLFUNC(146); CALLFUNC(147); CALLFUNC(148); CALLFUNC(149);
	CALLFUNC(150); CALLFUNC(151); CALLFUNC(152); CALLFUNC(153); CALLFUNC(154);
	CALLFUNC(155); CALLFUNC(156); CALLFUNC(157); CALLFUNC(158); CALLFUNC(159);
	CALLFUNC(160); CALLFUNC(161); CALLFUNC(162); CALLFUNC(163); CALLFUNC(164);
	CALLFUNC(165); CALLFUNC(166); CALLFUNC(167); CALLFUNC(168); CALLFUNC(169);
	CALLFUNC(170); CALLFUNC(171); CALLFUNC(172); CALLFUNC(173); CALLFUNC(174);
	CALLFUNC(175); CALLFUNC(176); CALLFUNC(177); CALLFUNC(178); CALLFUNC(179);
	CALLFUNC(180); CALLFUNC(181); CALLFUNC(182); CALLFUNC(183); CALLFUNC(184);
	CALLFUNC(185); CALLFUNC(186); CALLFUNC(187); CALLFUNC(188); CALLFUNC(189);
	CALLFUNC(190); CALLFUNC(191); CALLFUNC(192); CALLFUNC(193); CALLFUNC(194);
	CALLFUNC(195); CALLFUNC(196); CALLFUNC(197); CALLFUNC(198); CALLFUNC(199);
	CALLFUNC(200); CALLFUNC(201); CALLFUNC(202); CALLFUNC(203); CALLFUNC(204);
	CALLFUNC(205); CALLFUNC(206); CALLFUNC(207); CALLFUNC(208); CALLFUNC(209);
	CALLFUNC(210); CALLFUNC(211); CALLFUNC(212); CALLFUNC(213); CALLFUNC(214);
	CALLFUNC(215); CALLFUNC(216); CALLFUNC(217); CALLFUNC(218); CALLFUNC(219);
	CALLFUNC(220); CALLFUNC(221); CALLFUNC(222); CALLFUNC(223); CALLFUNC(224);
	CALLFUNC(225); CALLFUNC(226); CALLFUNC(227); CALLFUNC(228); CALLFUNC(229);
	CALLFUNC(230); CALLFUNC(231); CALLFUNC(232); CALLFUNC(233); CALLFUNC(234);
	CALLFUNC(235); CALLFUNC(236); CALLFUNC(237); CALLFUNC(238); CALLFUNC(239);
	CALLFUNC(240); CALLFUNC(241); CALLFUNC(242); CALLFUNC(243); CALLFUNC(244);
	CALLFUNC(245); CALLFUNC(246); CALLFUNC(247); CALLFUNC(248); CALLFUNC(249);
	CALLFUNC(250); CALLFUNC(251); CALLFUNC(252); CALLFUNC(253); CALLFUNC(254);
	CALLFUNC(255); CALLFUNC(256); CALLFUNC(257); CALLFUNC(258); CALLFUNC(259);
	CALLFUNC(260); CALLFUNC(261); CALLFUNC(262); CALLFUNC(263); CALLFUNC(264);
	CALLFUNC(265); CALLFUNC(266); CALLFUNC(267); CALLFUNC(268); CALLFUNC(269);
	CALLFUNC(270); CALLFUNC(271); CALLFUNC(272); CALLFUNC(273); CALLFUNC(274);
	CALLFUNC(275); CALLFUNC(276); CALLFUNC(277); CALLFUNC(278); CALLFUNC(279);
	CALLFUNC(280); CALLFUNC(281); CALLFUNC(282); CALLFUNC(283); CALLFUNC(284);
	CALLFUNC(285); CALLFUNC(286); CALLFUNC(287); CALLFUNC(288); CALLFUNC(289);
	CALLFUNC(290); CALLFUNC(291); CALLFUNC(292); CALLFUNC(293); CALLFUNC(294);
	CALLFUNC(295); CALLFUNC(296); CALLFUNC(297); CALLFUNC(298); CALLFUNC(299);
}
