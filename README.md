Самиздат утилита make
======================== 
***Это работа Бельской Александры*** 

### Для сборки необходимы:
     std (lib)
     make

#### Для сборки выполнить: 
     make clean 
     make all 

#### Для тестирования выполнить:
     ./myMake -n -d 5 -f testMake all
 __здесь__:
* -n - не выболнять, а только разобрать и показать процесс
* -d 5 - уровень отладочных сообщений ( 5 - самый детальный )
* -f [имя файла] - имя Make-файла (по умолчанию Makefile)
* [последний параметр] - имя выполняемой цели

#### ЧТО ОСТАЛОСЬ СДЕЛАТЬ:
*  обработка конструкций %.o : %.cpp  и  $^ / $@
*  написать help
 
