# ALL (Assembly-Like Language)
_the language still have a lot of bugs and limited functionalities, so do not expect something incredible._

## Launch a file
```shell
gcc main.c -o main # compile
./main # execute
```

# Docs

## Registers
There are 8 registers:
- rax, rbx, rcx, rdx, rex, rfx (for storing values)
- rip (instruction pointer : which line is executed)
- rmp (memory pointer : which memory place is pointed)

## Types
ALL is dynamicly typed. Available types are:
- integer
- float
- string

Others types (arrays, dictionaries...) can be done using the [memory](#memory).

## Basic instructions

#### init
_set a register to a specific value_
```all
init rax 4
```

#### print
_show value of a register_
```all
print 4
```

#### input
_get string from user input and store the value in a register_
```all
input rax
```

#### add, sub, mul and div
_basic operations on registers_
```all
add rax 4
sub rax 2
mul rax 3
div rax 2
print rax # result : 3
```

#### exit
_close the program_
```all
print 'end'
exit
print "Hello World!" # not printed
```

#### comments
_oneline comments_
```all
# hello
```

## If statement

#### go?eq, go?bi, go?le
_go to a specific line if the value equals, is bigger or less than 0_
```all
init rax 3
go?bi rax 4
print "hello" # not executed
print 'end'
```

## Convertion

#### tos
_converts a register which contains an integer or a float to a string_
```all
init rex 3
tos rex # rex now contains a string
```

#### toi
_converts a register which contains a string or a float to a integer_
```all
init rex '3'
toi rex # rex now contains an integer
```

#### tof
_converts a register which contains an integer or a string to a float_
```all
init rex 3
tof rex # rex now contains a float
```

## Memory

#### store
_store a value where rmp is pointing_
```all
store 5 # value 5 is stored at case number 0
add rmp 2 # case number 2
init rax rmp
store rax # value 2 is stored
```

#### load 
_load in a register the value where rmp is pointing_
```all
store 3
load rax
print rax # result : 3
```
