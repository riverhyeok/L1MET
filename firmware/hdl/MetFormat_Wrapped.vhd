library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library IO;
use IO.DataType.all;
use IO.ArrayTypes.all;

library MetCtrlToken;
use MetCtrlToken.DataType;

library MetFormat;

entity MetFormat_Wrapped is
port(
    clk : in std_logic;
    d   : in tData  := cNull;
    q   : out tData  := cNull;
    token_d : in  MetCtrlToken.DataType.tData := MetCtrlToken.DataType.cNull;
    token_q : out MetCtrlToken.DataType.tData := MetCtrlToken.DataType.cNull
);
end MetFormat_Wrapped;

architecture rtl of MetFormat_Wrapped is
    -- signals to reattach the valid bits
    signal token_i : std_logic_vector(MetCtrlToken.DataType.tData'size-1 downto 0) := (others => '0');
    signal start : std_logic := '0';
    signal done  : std_logic := '0';
begin

    HLSIP : entity MetFormat.met_format
    port map(
        ap_clk   => clk,
        ap_rst   => '0',
        ap_start => start,
        ap_done => done,
        d        => d.data(38 downto 0),
        q      => q.data(63 downto 0),
        token_d  => MetCtrlToken.DataType.ToStdLogicVector(token_d),
        token_q => token_i
    );

    start <= '1' when d.DataValid else '0';
    q.DataValid <= True when done = '1' else False;
    q.FrameValid <= True when done = '1' else False;


end rtl;