# CI_LISP

All Debugging and Issues across each task will be documented here

#Task 1 
 - All methods tested and acceptable for task 1 
 - Currently no error prints for bad number of operands for functions

#Task 2
 - Sample runs match lab description sample runs
 - All testing shows that everything should be working fine for now and casting properly
 - Still no warning or error prints yet
 - Haven't handled events where multiple symbols in the same scope are defined
 - Haven't handled events where an undeclared symbol is used
 
#Task 3
 - All forms of numType casting work on both number literals and symbols
 - Design Choice: If you cast a Double to an Integer a warning message will be displayed, but the symbol will be treated as a integer for the remainder of it's binding, unless recast at a later point.
 - Can't re-cast a symbol if it isn't in a let section (Although that functionality isn't included in the grammar)
 
 #Task 4
 - All printing is working as it should
 - You can also use (print (print (print....) even though that's silly
 - Shouldn't really need any debugging until maybe Task 5 since the function args change
 
 #Task 5
 - Code passes all test cases included in the new instructions file for Task1 and also passes for everything in the quiz
 - All things are tested and working from all tasks
 - Error messages are currently passed through yyerror, thus they are suppressed. May consider switching stream to stdout or not using yyerror for error messages releated to functions

#Task 6
- Got all error and warning mesasges that I can think of working
- All new functions are working
- Design Implementation: Less, Greater and Equal only return INTS
- Multiple Symbol definitions with the same ID now defaults to the most recently assigned node
- I hope I'm trying hard enough to break my code. Only time will tell.

#Task 7 + 8
-I couldn't get my custom functions to stop SEG-FAULTING for part 9 and I was working up to 12AM so I wasn't able to get this task working.
-I'm also not sure if my free works completely as I was not able to test it thouroughly, but it seems to free properly in most of the scenarios I was able to test
- Everything up to Task 7/8 should work perfectly and I even implemented extra stuff according to what you mentioned in the new Task 1 instructions. If anything does break I just wasn't able to find a test case for it.
