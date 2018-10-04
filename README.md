# compiler-60min

### Грамматика языка
<Program>       -> <FunctionList> EOF  
<FunctionList>  -> <Function> <FunctionList>  
<FunctionList>  -> #Eps#  
<Function>      -> FUN IDENTIFIER LPAREN <ParamList> RPAREN ARROW <Type> COLON <Statement>  
<ParamList>     -> <Param> <TailParamList>  
<ParamList>     -> #Eps#  
<TailParamList> -> COMMA <Param> <TailParamList>  
<TailParamList> -> #Eps#  
<Param>         -> IDENTIFIER COLON <Type>  
<Type>          -> INT  
<Type>          -> FLOAT  
<Type>          -> BOOL  
<Type>          -> ARRAY LABRACKET <Type> RABRACKET  
<Statement>     -> <Condition>  
<Statement>     -> <Loop>  
<Statement>     -> <Decl>  
<Statement>     -> <Assign>  
<Statement>     -> <Return>  
<Statement>     -> <Composite>  
<Condition>     -> IF LPAREN <Expression> RPAREN <Statement> <OptionalElse>  
<OptionalElse>  -> ELSE <Statement>  
<OptionalElse>  -> #Eps#  
<Loop>          -> WHILE LPAREN <Expression> RPAREN <Statement>  
<Decl>          -> VAR IDENTIFIER COLON <Type> SEMICOLON  
<Assign>        -> IDENTIFIER ASSIGN <Expression> SEMICOLON  
<Return>        -> RETURN <Expression> SEMICOLON  
<Composite>     -> LCURLY <StatementList> RCURLY  
<StatementList> -> <Statement> <StatementList>  
<StatementList> -> #Eps#  
<Expression>    -> IDENTIFIER  
<Expression>    -> INTLITERAL  
<Expression>    -> FLOATLITERAL  
<Expression>    -> TRUE  
<Expression>    -> FALSE
