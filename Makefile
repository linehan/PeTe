#########################
# Configure build      
#########################

CPP_COMPILER=gcc
ASM_COMPILER=yasm

#
#  optimize  
#  level 3    
#         \    
CC_FLAGS=-O4 -s -fomit-frame-pointer -DUNIX #-ffast-math 
LD_FLAGS=-lm -pthread
#	  /    
#      math   
#
# NOTE on -ffast-math
#
# First, breaks strict IEEE compliance, e.g. allows re-ordering of 
# instructions to a mathematical equivalent, which may not be IEEE
# floating-point equivalent. 
#
# Second, disables setting errno after single-instruction math functions, 
# avoiding a write to a thread-local variable (can produce 100% speedup on
# certain architectures). 
#
# Third, assumes finite math only, meaning no checks for NaN (or 0) are 
# made where they would normally be. It is assumed these values will never 
# appear. 
#
# Fourth, enables reciprocal approximations for division and reciprocal 
# square root.
#
# Fifth, disables signed zero (even if the compile target supports it) 
# and rounding math, which enables optimizations e.g. constant folding.
#
# Sixth, generates code assuming no hardware interrupts occur in math
# due to signal()/trap(). If these cannot be disabled on the compile
# target and consequently occur, they will not be handled.
#

#########################
# Configure files 
#########################

PROG_SOURCES=main.c perm.c hashtable.c pset.c io.c ptable.c

PROG_OBJECTS=$(PROG_SOURCES:.c=.o)


#########################
# Configure rules 
#########################

all: p 

p: $(PROG_SOURCES) 
	$(CPP_COMPILER) $(CC_FLAGS) $(PROG_SOURCES) -o p $(LD_FLAGS)

clean:
	rm -f $(PROG_OBJECTS) p gmon.out 
