-- uart.vhd: UART controller - receiving part
-- Author(s): 
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------
entity UART_RX is
port(	
    CLK: 	    in std_logic;
	RST: 	    in std_logic;
	DIN: 	    in std_logic;
	DOUT: 	    out std_logic_vector(7 downto 0);
	DOUT_VLD:   out std_logic := '0'
);
end UART_RX;  

-------------------------------------------------
architecture behavioral of UART_RX is

signal clock_cnt    : std_logic_vector(4 downto 0);
signal bit_cnt      : std_logic_vector(3 downto 0);
signal read_on      : std_logic;
signal clk_cnt_on   : std_logic;
signal valid_bit    : std_logic;

begin

	FSM: entity work.UART_FSM(behavioral)
	port map(
		CLK             => CLK,
		RST             => RST,
		DATA            => DIN,
		READ_ON         => read_on,
		VALID_BIT       => valid_bit,
		CLK_CNT_ON      => clk_cnt_on,
		END_READ        => bit_cnt(3),
		CLOCK_CNT       => clock_cnt
	);

	DOUT_VLD <= valid_bit;
	process(CLK) begin
		if (CLK'event AND CLK = '1') then
			if (RST = '1') then
				 clock_cnt <= "00000";
				 bit_cnt <= "0000";
			else
				 if (clk_cnt_on = '1') then
					  clock_cnt <= clock_cnt + 1;
				 else
					  clock_cnt <= "00000";
				 end if;
				 if (read_on = '1' and clock_cnt(4) = '1') then
					  DOUT(conv_integer(bit_cnt)) <= DIN;
					  bit_cnt <= bit_cnt + 1;
					  clock_cnt <= "00001";
				 end if;
				 if (read_on = '0') then
					  bit_cnt <= "0000";
				 end if;
			end if;
		end if;
	end process;
end behavioral;
