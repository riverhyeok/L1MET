library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library IO;
use IO.DataType.all;
use IO.ArrayTypes.all;

library METCtrlToken;
use METCtrlToken.DataType;

library PuppiMETxy;

use work.PkgConstants.all;

entity PuppiMETxy_Wrapped is
port(
    clk : in std_logic;
    in_particles : in Vector(0 to NPARTICLES-1) := NullVector(NPARTICLES);
    met_xy : out tData := cNull;
    token_d : in METCtrlToken.DataType.tData;
    token_q : out METCtrlToken.DataType.tData
);
end PuppiMETxy_Wrapped;

architecture rtl of PuppiMETxy_Wrapped is
    signal token_i : std_logic_vector(METCtrlToken.DataType.tData'size-1 downto 0) := (others => '0');
    signal start : std_logic := '0';
    signal done  : std_logic := '0';
    signal met_xy_raw : std_logic_vector(63 downto 0) := (others => '0');
begin

    HLSIP : entity PuppiMETxy.puppimet_xy
    port map(
    ap_clk => clk,
    ap_rst => '0',
    ap_start => start,
    ap_done => done,
    in_particles_0 => in_particles(0).data(36 downto 0),
    in_particles_1 => in_particles(1).data(36 downto 0),
    in_particles_2 => in_particles(2).data(36 downto 0),
    in_particles_3 => in_particles(3).data(36 downto 0),
    in_particles_4 => in_particles(4).data(36 downto 0),
    in_particles_5 => in_particles(5).data(36 downto 0),
    in_particles_6 => in_particles(6).data(36 downto 0),
    in_particles_7 => in_particles(7).data(36 downto 0),
    in_particles_8 => in_particles(8).data(36 downto 0),
    in_particles_9 => in_particles(9).data(36 downto 0),
    in_particles_10 => in_particles(10).data(36 downto 0),
    in_particles_11 => in_particles(11).data(36 downto 0),
    in_particles_12 => in_particles(12).data(36 downto 0),
    in_particles_13 => in_particles(13).data(36 downto 0),
    in_particles_14 => in_particles(14).data(36 downto 0),
    in_particles_15 => in_particles(15).data(36 downto 0),
    in_particles_16 => in_particles(16).data(36 downto 0),
    in_particles_17 => in_particles(17).data(36 downto 0),
    in_particles_18 => in_particles(18).data(36 downto 0),
    in_particles_19 => in_particles(19).data(36 downto 0),
    in_particles_20 => in_particles(20).data(36 downto 0),
    in_particles_21 => in_particles(21).data(36 downto 0),
    in_particles_22 => in_particles(22).data(36 downto 0),
    in_particles_23 => in_particles(23).data(36 downto 0),
    in_particles_24 => in_particles(24).data(36 downto 0),
    in_particles_25 => in_particles(25).data(36 downto 0),
    in_particles_26 => in_particles(26).data(36 downto 0),
    in_particles_27 => in_particles(27).data(36 downto 0),
    in_particles_28 => in_particles(28).data(36 downto 0),
    in_particles_29 => in_particles(29).data(36 downto 0),
    in_particles_30 => in_particles(30).data(36 downto 0),
    in_particles_31 => in_particles(31).data(36 downto 0),
    in_particles_32 => in_particles(32).data(36 downto 0),
    in_particles_33 => in_particles(33).data(36 downto 0),
    in_particles_34 => in_particles(34).data(36 downto 0),
    in_particles_35 => in_particles(35).data(36 downto 0),
    in_particles_36 => in_particles(36).data(36 downto 0),
    in_particles_37 => in_particles(37).data(36 downto 0),
    in_particles_38 => in_particles(38).data(36 downto 0),
    in_particles_39 => in_particles(39).data(36 downto 0),
    in_particles_40 => in_particles(40).data(36 downto 0),
    in_particles_41 => in_particles(41).data(36 downto 0),
    in_particles_42 => in_particles(42).data(36 downto 0),
    in_particles_43 => in_particles(43).data(36 downto 0),
    in_particles_44 => in_particles(44).data(36 downto 0),
    in_particles_45 => in_particles(45).data(36 downto 0),
    in_particles_46 => in_particles(46).data(36 downto 0),
    in_particles_47 => in_particles(47).data(36 downto 0),
    in_particles_48 => in_particles(48).data(36 downto 0),
    in_particles_49 => in_particles(49).data(36 downto 0),
    in_particles_50 => in_particles(50).data(36 downto 0),
    in_particles_51 => in_particles(51).data(36 downto 0),
    in_particles_52 => in_particles(52).data(36 downto 0),
    in_particles_53 => in_particles(53).data(36 downto 0),
    in_particles_54 => in_particles(54).data(36 downto 0),
    in_particles_55 => in_particles(55).data(36 downto 0),
    in_particles_56 => in_particles(56).data(36 downto 0),
    in_particles_57 => in_particles(57).data(36 downto 0),
    in_particles_58 => in_particles(58).data(36 downto 0),
    in_particles_59 => in_particles(59).data(36 downto 0),
    in_particles_60 => in_particles(60).data(36 downto 0),
    in_particles_61 => in_particles(61).data(36 downto 0),
    in_particles_62 => in_particles(62).data(36 downto 0),
    in_particles_63 => in_particles(63).data(36 downto 0),
    in_particles_64 => in_particles(64).data(36 downto 0),
    in_particles_65 => in_particles(65).data(36 downto 0),
    in_particles_66 => in_particles(66).data(36 downto 0),
    in_particles_67 => in_particles(67).data(36 downto 0),
    in_particles_68 => in_particles(68).data(36 downto 0),
    in_particles_69 => in_particles(69).data(36 downto 0),
    in_particles_70 => in_particles(70).data(36 downto 0),
    in_particles_71 => in_particles(71).data(36 downto 0),
    in_particles_72 => in_particles(72).data(36 downto 0),
    in_particles_73 => in_particles(73).data(36 downto 0),
    in_particles_74 => in_particles(74).data(36 downto 0),
    in_particles_75 => in_particles(75).data(36 downto 0),
    in_particles_76 => in_particles(76).data(36 downto 0),
    in_particles_77 => in_particles(77).data(36 downto 0),
    in_particles_78 => in_particles(78).data(36 downto 0),
    in_particles_79 => in_particles(79).data(36 downto 0),
    in_particles_80 => in_particles(80).data(36 downto 0),
    in_particles_81 => in_particles(81).data(36 downto 0),
    in_particles_82 => in_particles(82).data(36 downto 0),
    in_particles_83 => in_particles(83).data(36 downto 0),
    in_particles_84 => in_particles(84).data(36 downto 0),
    in_particles_85 => in_particles(85).data(36 downto 0),
    in_particles_86 => in_particles(86).data(36 downto 0),
    in_particles_87 => in_particles(87).data(36 downto 0),
    in_particles_88 => in_particles(88).data(36 downto 0),
    in_particles_89 => in_particles(89).data(36 downto 0),
    in_particles_90 => in_particles(90).data(36 downto 0),
    in_particles_91 => in_particles(91).data(36 downto 0),
    in_particles_92 => in_particles(92).data(36 downto 0),
    in_particles_93 => in_particles(93).data(36 downto 0),
    in_particles_94 => in_particles(94).data(36 downto 0),
    in_particles_95 => in_particles(95).data(36 downto 0),
    in_particles_96 => in_particles(96).data(36 downto 0),
    in_particles_97 => in_particles(97).data(36 downto 0),
    in_particles_98 => in_particles(98).data(36 downto 0),
    in_particles_99 => in_particles(99).data(36 downto 0),
    in_particles_100 => in_particles(100).data(36 downto 0),
    in_particles_101 => in_particles(101).data(36 downto 0),
    in_particles_102 => in_particles(102).data(36 downto 0),
    in_particles_103 => in_particles(103).data(36 downto 0),
    in_particles_104 => in_particles(104).data(36 downto 0),
    in_particles_105 => in_particles(105).data(36 downto 0),
    in_particles_106 => in_particles(106).data(36 downto 0),
    in_particles_107 => in_particles(107).data(36 downto 0),
    in_particles_108 => in_particles(108).data(36 downto 0),
    in_particles_109 => in_particles(109).data(36 downto 0),
    in_particles_110 => in_particles(110).data(36 downto 0),
    in_particles_111 => in_particles(111).data(36 downto 0),
    in_particles_112 => in_particles(112).data(36 downto 0),
    in_particles_113 => in_particles(113).data(36 downto 0),
    in_particles_114 => in_particles(114).data(36 downto 0),
    in_particles_115 => in_particles(115).data(36 downto 0),
    in_particles_116 => in_particles(116).data(36 downto 0),
    in_particles_117 => in_particles(117).data(36 downto 0),
    in_particles_118 => in_particles(118).data(36 downto 0),
    in_particles_119 => in_particles(119).data(36 downto 0),
    in_particles_120 => in_particles(120).data(36 downto 0),
    in_particles_121 => in_particles(121).data(36 downto 0),
    in_particles_122 => in_particles(122).data(36 downto 0),
    in_particles_123 => in_particles(123).data(36 downto 0),
    in_particles_124 => in_particles(124).data(36 downto 0),
    in_particles_125 => in_particles(125).data(36 downto 0),
    in_particles_126 => in_particles(126).data(36 downto 0),
    in_particles_127 => in_particles(127).data(36 downto 0),
    
    met_xy => met_xy_raw(43 downto 0),
    token_d => METCtrlToken.DataType.ToStdLogicVector(token_d),
    token_q => token_i
    );
    
    start <= '1' when token_d.DataValid else '0';
    met_xy.DataValid <= True when done = '1' else False;
    met_xy.FrameValid <= True when done = '1' else False;
    met_xy.data <= met_xy_raw when met_xy.DataValid else (others => '0');
    token_q <= METCtrlToken.DataType.ToDataType(token_i);

end rtl;

