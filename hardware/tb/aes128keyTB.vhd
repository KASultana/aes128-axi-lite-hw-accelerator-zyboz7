--AES core testbench

library ieee;
use ieee.std_logic_1164.all;

entity aes128keyTB is
end aes128keyTB;

architecture behavior of aes128keyTB is

    component aes128key
        port(
            reset  : in  std_logic;
            clock  : in  std_logic;
            empty  : out std_logic;
            load   : in  std_logic;
            key    : in  std_logic_vector(127 downto 0);
            plain  : in  std_logic_vector(127 downto 0);
            ready  : out std_logic;
            cipher : out std_logic_vector(127 downto 0)
        );
    end component;

    signal reset  : std_logic := '0';
    signal clock  : std_logic := '0';
    signal load   : std_logic := '0';
    signal key    : std_logic_vector(127 downto 0) := (others => '0');
    signal plain  : std_logic_vector(127 downto 0) := (others => '0');
    signal empty  : std_logic;
    signal ready  : std_logic;
    signal cipher : std_logic_vector(127 downto 0);

    constant clock_period : time := 10 ns;

begin
    uut : aes128key
        port map (
            reset  => reset,
            clock  => clock,
            empty  => empty,
            load   => load,
            key    => key,
            plain  => plain,
            ready  => ready,
            cipher => cipher
        );

    -- clock
    clock_process : process
    begin
        clock <= '0';
        wait for clock_period/2;
        clock <= '1';
        wait for clock_period/2;
    end process;

    -- stimulus
    stim_proc : process
    begin
        reset <= '1';
        wait for 100 ns;
        reset <= '0';

        wait for clock_period*10;

        -- FIPS-197 AES-128 test vector
        key   <= x"000102030405060708090a0b0c0d0e0f";
        plain <= x"00112233445566778899aabbccddeeff";
        load  <= '1';
        wait for clock_period;
        load  <= '0';

        -- wait long enough for all rounds in this core
        wait for clock_period*80;

        report "Simulation finished, cipher = " severity note;
        wait;
    end process;

end behavior;


