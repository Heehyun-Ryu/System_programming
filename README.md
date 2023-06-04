# System_programming
term_project

 <Yachtdice game>

c언어를 활용해서 만든 Yachtdice 게임. 

•Ubuntu 20.04 LTS에서 구현했습니다. 

•Yachtdice_client.c는 curses library를 사용했기 때문에 컴파일 시 -lcurses를 사용해야만 합니다.

•Yachtdice_server.c와Yachtdice_client.c는 50000포트를 사용합니다.

• Yachtdice_server.c - 멀티모드를 하려면 Yachtdice_client 프로그램보다 선행되어야 하는 프로그램. Yachtdice_client 간의 통신을 할 수 있도록 하고, 최대 2인까지 접속이 가능합니다.

• Yachtdice_client.c - Yachtdice 게임을 실행하는 프로그램. 싱글모드 멀티모드로 나뉘어져 있다. 싱글 모드에서는 사용법과 룰을 익힐 수 있도록 혼자하는 모드이고 멀티모드는 다른 사람과 1:1로 통신하며 대전을 하는 모드

- Yachtdice_client를 실행하면 싱글모드와 멀티모드를 선택할 수 있는 화면이 나타납니다. W,S로 위아래로 움직이며 모드를 선택할 수 있습니다. 스페이스바를 누르면 선택이 가능합니다. 

- 멀티모드를 사용하는 경우 상대방의 IP주소를 입력해야합니다.

- 게임의 순서는 먼저 접속한 사람이 먼저 게임을 진행하게 됩니다.

- 주사위를 멈추는 키는 스페이스바. 주사위를 선택할 때는 왼쪽부터 1,2,3,4,5번 주사위이고 바꿀 주사위의 번호를 띄어쓰기로 구분해서 입력하면 됩니다. 

- 주사위가 마음에 들면 ok를 입력하면 되고, 전체적으로 주사위를 다시 돌리고 싶으면 no를 입력하면 됩니다. 

- 주사위를 통해 족보가 생성되면 얻을 점수를 선택합니다.
