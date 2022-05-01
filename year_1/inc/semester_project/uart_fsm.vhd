-- uart_fsm.vhd: UART controller - finite state machine
-- Author(s): 
--
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------
entity UART_FSM is
port(
   CLK: in std_logic;
   RST: in std_logic;
   CLOCK_CNT: in std_logic_vector(4 downto 0);
   CLK_CNT_ON: out std_logic;
   DATA: in std_logic;
   END_READ: in std_logic;
   READ_ON: out std_logic;
   VALID_BIT: out std_logic
   );

end entity UART_FSM;

-------------------------------------------------
architecture behavioral of UART_FSM is
type STATE is (START, BEGIN_READ, READING, READ_END, VALIDATE);
signal st : STATE := START;
begin

   CLK_CNT_ON <= '0' when ((st = VALIDATE) or (st = START)) else '1';
   READ_ON <= '1' when (st = READING) else '0';
   VALID_BIT <= '1' when (st = VALIDATE) else '0';
   
   process (CLK) begin
      if (CLK'event AND CLK = '1') then
			if (RST = '0') then
				case st is
				
					when START => 
						if (DATA = '0') then
							st <= BEGIN_READ;
						end if;
					when BEGIN_READ =>
						if (CLOCK_CNT = "11000") then
							st <= READING;
						end if;
					when READING =>
						if (END_READ = '1') then
							st <= READ_END;
						end if;
					when READ_END =>
						if (CLOCK_CNT = "10000") then
							st <= VALIDATE;
						end if;
					when VALIDATE =>
						st <= START;
						
				end case;
			else
				st <= START;
			end if;
      end if;
   end process;
end behavioral;
