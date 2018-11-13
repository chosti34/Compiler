# compiler-60min

### Грамматика языка
<Program>        -> <FunctionList> EndOfFile
<FunctionList>   -> <Function> <FunctionList>
<FunctionList>   -> #Eps#
<Function>       -> Func Identifier LeftParenthesis <ParamList> RightParenthesis Arrow <Type> Colon <Statement>
<ParamList>      -> <Param> <ParamListTail>
<ParamList>      -> #Eps#
<ParamListTail>  -> Comma <Param> <ParamListTail>
<ParamListTail>  -> #Eps#
<Param>          -> Identifier Colon <Type>
<Type>           -> Integer
<Type>           -> Float
<Type>           -> Bool
<Type>           -> Array LeftBracket <Type> RightBracket
<Statement>      -> <Condition>
<Statement>      -> <Loop>
<Statement>      -> <Decl>
<Statement>      -> <Assign>
<Statement>      -> <Return>
<Statement>      -> <Composite>
<Condition>      -> If LeftParenthesis <Expression> RightParenthesis <Statement> <OptionalElse>
<OptionalElse>   -> Else <Statement>
<OptionalElse>   -> #Eps#
<Loop>           -> While LeftParenthesis <Expression> RightParenthesis <Statement>
<Decl>           -> Var Identifier Colon <Type> Semicolon
<Assign>         -> Identifier Assign <Expression> Semicolon
<Return>         -> Return <Expression> Semicolon
<Composite>      -> LeftCurly <StatementList> RightCurly
<StatementList>  -> <Statement> <StatementList>
<StatementList>  -> #Eps#
<Expression>     -> <AddSubExpr>
<AddSubExpr>     -> <MulDivExpr> <AddSubExprTail>
<AddSubExprTail> -> Plus <MulDivExpr> <AddSubExprTail>
<AddSubExprTail> -> Minus <MulDivExpr> <AddSubExprTail>
<AddSubExprTail> -> #Eps#
<MulDivExpr>     -> <AtomExpr> <MulDivExprTail>
<MulDivExprTail> -> Mul <AtomExpr> <MulDivExprTail>
<MulDivExprTail> -> Div <AtomExpr> <MulDivExprTail>
<MulDivExprTail> -> #Eps#
<AtomExpr>       -> IntegerConstant
<AtomExpr>       -> FloatConstant
<AtomExpr>       -> TrueConstant
<AtomExpr>       -> FalseConstant
<AtomExpr>       -> StringConstant
<AtomExpr>       -> LeftParenthesis <Expression> RightParenthesis
