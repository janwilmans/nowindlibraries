
/* Code Style Guidelines for Nowind Libraries, these are guidelines, not rules
 * if you have good reason to use a different style, go ahead, but if in doubt, follow these guidelines.
 *
 * Variables: 
 * int aCounter;  //method argument, rationale: improves readability, this variable has just a local scope, but has been initialized outside the method
 * int lCounter;  //local variable, rationale: improves readability, this variable has just a local scope
 * int mCounter;  //member variable, rationale: improves readability, this variable will be available in the next method call
 * names: try to be descriptive and specific where possible, ie. if and integer is a length or a height,
 *        dont call it a lSize
 *        
 *
 * avoid the use of: 
 *  char *lFoo, char * lFoo, char*lFoo, or char&lFoo
 *  char *lFoo, lBar = 0;
 * instead use:
 *  char* lFoo, char* lFoo and char& lFoo;
 *  char* lFoo;
 *  char* lBar = 0;
 * rationale: the * and & symbols are part of the type (in the context of declarations)
 * so they should not be stuck to the variable name
 *
 * method names: "camelBackStyle", rationale: none, conforming to http://geosoft.no/development/cppstyle.html
 * Try to use a verb-noun pair, unless the method operates on its containing class, in which case, use just a verb.
 * ex: RenderScreen(), CalculateOdds(), Initialize()
 *
 * placement of {} is free, but it recommened to use {} for any if-statement that
 * might otherwise be mis-interpreted.
 *
 * Exceptions: 
 * - short local variable names like: for(int i=0;...) are preferred if it improves readability, 
 *   rationale: shortens code, making it easier to read
 * 
 * Class names: 
 * choose a singular noun and avoid verbs unless there is a good justification to do otherwise
 * bad: Products, Render. good: Product, RenderEngine
 *
 *
 * considerations: 
 * when to start left curly brace { on a new line  ?
 * use strict indentation: 4 spaces, no tabs
 * indent comment at the same line as the code 
 * declare local variables as close as possible to the first time they're used 
 * file names should reflect the class that they contain, contain a single class, and have the
 * same case (non win32 compatibility)
 *
 * pitfalls:
 *
 * never use abstract names:
 * 
 * void DoThis();
 * Routine48();
 * int ZimboVariable;
 *
 * never use acronyms, they are unreadable to others:
 * 
 * //AcronymFunction
 * AF();
 * //SuperFastAcronymFunction
 * SFAF()
 * 
 * 
 */

