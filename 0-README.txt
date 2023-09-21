Initial creation of the GCC source:

  cd
  mkdir z-gcc
  git clone 'https://github.com/gcc-mirror/gcc.git'

Figure out which version to checkout:

  cd ~/z-gcc/gcc
  git log --no-walk --tags --pretty="%H %ci %d" --decorate=full
  # Take a note of the tags for major point-versions e.g. "releses/gcc-8"

Get desired version and create build directory:

  cd ~/z-gcc/gcc
  git checkout 4212a6a3e44
  b  # back in ~/z-gcc
  mv gcc src
  cd src
  ./contrib/download_prerequisites
  b  # back in ~/z-gcc
  mkdir build

Get my customisation files:

  g phd ; cd gcc-reorder
  ./sync

To do a full build:

  cd ~/z-gcc
  cd build
  cls ; ../make1 -s ; head -15 ../p1.txt

To do a quick build and test:

  cd ~/z-gcc/build
  cls ; ../make1 ; head -15 ../p1.txt

To just do a test with no build:

  cd ~/z-gcc/build
  cls ; ../make1 -nm ; head -15 ../p1.txt

Editing source code:

  cd ~/z-gcc/src/gcc
  e passes.c

Sync source code:

  g phd
  cd gcc-reorder
  bmanisync # to sync with ~/z-gcc
  # or:
  # bmanisync -with gcc9
  bmd .

Inlining/LICM bug:

  cd ~/z-gcc/build
  ./bench1
  #
  # Or to run tests manually:
  # e haj-ali-3.c 
  # gcc -O2 haj-ali-3.c -lm -o ha && time ./ha
  # gcc -O1 haj-ali-3.c -lm -o ha && time ./ha