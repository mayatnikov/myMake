#!/bin/bash
#  Создание тестового проекта
echo делаем тестовые исходники
#
TESTPROJECT=../test-project
THISDIR=$PWD
#
mkdir -p $TESTPROJECT/core
cp testMakefile testInclude badMake $TESTPROJECT
cat << 'EOF' > $TESTPROJECT/core/graph.c
  int graph() {return 0;}
EOF
cat << 'EOF' > $TESTPROJECT/core/str_operation.cpp 
  int str_operation() {return 0;}
EOF
cat << 'EOF' > $TESTPROJECT/core/simple_list.c 
  int simple_list() {return 0;}
EOF
cat << 'EOF' > $TESTPROJECT/core/my_malloc.c 
  int my_malloc() {return 0;}
EOF
cat << 'EOF' > $TESTPROJECT/core/c_mpi_writer.cpp 
  int c_mpi_writer() {return 0;}
EOF
cat << 'EOF' > $TESTPROJECT/core/graph2c++.c 
  int main() {return 0;}
EOF
#
echo  делаем библиотеку lib1.a
#
mkdir -p $TESTPROJECT/libs
cat << 'EOF' > $TESTPROJECT/libs/modul1.c 
  int funclib1() { return 1; }
EOF
cat << 'EOF' > $TESTPROJECT/libs/modul2.c 
  int funclib2() { return 1; }
EOF
gcc -Wall -c $TESTPROJECT/libs/modul1.c
gcc -Wall -c $TESTPROJECT/libs/modul2.c
ar rcs $TESTPROJECT/libs/lib1.a modul1.o modul2.o

echo смотрим что получилось
ls -alR $TESTPROJECT

cat << EOF
**********************************
** Тестовый проект создан 
** для проверки работоспособности
** 1 - соберите mymake, команда: make all
** 2 - перейти в директорию $TESTPROJECT
** 3 - запустить $THISDIR/mymake -d 5 -f testMakefile install
** 4 - оценить результат исполнения:
**     посмотреть содержимое в: $TESTPROJECT $TESTPROJECT/distr/bin
*************************
EOF