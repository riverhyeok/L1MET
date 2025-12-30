library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library IO;
use IO.DataType.all;
use IO.ArrayTypes.all;

library METCtrlToken;
use METCtrlToken.DataType;

library PxPyToPtPhi;

use work.PkgConstants.all;

entity PxPyToPtPhi_Wrapped is
port(
    clk : in std_logic;
    met_xy : in tData := cNull;
    out_met : out tdata := cNull;
    token_d : in METCtrlToken.DataType.tData;
    token_q : out METCtrlToken.DataType.tData
);
end PxPyToPtPhi_Wrapped;

architecture rtl of PxPyToPtPhi_Wrapped is
    signal token_i : std_logic_vector(METCtrlToken.DataType.tData'size-1 downto 0) := (others => '0');
    signal start : std_logic := '0';
    signal done  : std_logic := '0';
    signal out_met_raw : std_logic_vector(63 downto 0) := (others => '0');

begin

    HLSIP : entity PxPyToPtPhi.pxpy_to_ptphi
    port map(
    ap_clk => clk,
    ap_rst => '0',
    ap_start => start,
    ap_done => done,
    met_xy => met_xy.data(43 downto 0),
    out_met => out_met_raw(38 downto 0),
    token_d => METCtrlToken.DataType.ToStdLogicVector(token_d),
    token_q => token_i
    
    );

    start <= '1' when token_d.DataValid else '0';
    out_met.DataValid <= True when done = '1' else False;
    out_met.FrameValid <= True when done = '1' else False;
    out_met.data <= out_met_raw when out_met.DataValid else (others => '0');
    token_q <= METCtrlToken.DataType.ToDataType(token_i);

end rtl;

