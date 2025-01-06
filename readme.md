## Registers
There are 8 registers:
- rax, rbx, rcx, rdx, rex, rfx (for storing values)
- rip (instruction pointer : which line is executed)
- rmp (memory pointer : which memory place is pointed)

## Types
ASL is dynamicly typed. Available types are:
- integer
- float
- string

Others types (arrays, dictionaries...) can be done using the [memory](#memory).

## Basic instructions

#### init
_set a register to a specific value_
```asl
init rax 4
```

#### print
_show value of a register_
```asl
print 4
```

#### input
_get value from user input and store the value in a register_
```asl
input rax
```

#### add, sub, mul and div
_basic operations on registers_
```asl
add rax 4
sub rax 2
mul rax 3
div rax 2
print rax # result : 3
```

#### exit
_close the program_
```asl
print 'end'
exit
print "Hello World!" # not printed
```

#### comments
_oneline comments_
```asl
# hello
```

## If statement

#### go?eq, go?bi, go?le
_go to a specific line if the value equals, is bigger or less than 0_
```asl
init rax 3
go?bi rax 3
print "hello" # not executed
print 'end'
```
[!IMPORTANT]
the rip register begins to 0. when you want to go to the xth instruction, use number x-2 instead.


## Memory

#### store
_store a value where rmp is pointing_
```asl
store 5 # value 5 is stored at case number 0
add rmp 2 # case number 2
init rax rmp
store rax # value 2 is stored
```

#### load 
_load in a register the value where rmp is pointing_
```asl
store 3
load rax
print rax # result : 3
```