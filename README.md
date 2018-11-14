# compiler-60min

### Грамматика языка
<Program>        -> <FunctionList> EndOfFile
<FunctionList>   -> <Function> {CreateFunction} <FunctionList>
<FunctionList>   -> #Eps#
<Function>       -> Func Identifier LeftParenthesis <ParamList> RightParenthesis Arrow <Type> Colon <Statement>
<ParamList>      -> <Param> <ParamListTail>
<ParamList>      -> #Eps#
<ParamListTail>  -> Comma <Param> <ParamListTail>
<ParamListTail>  -> #Eps#
<Param>          -> Identifier Colon <Type>

<Type>           -> Integer {OnIntegerTypeParse}
<Type>           -> Float {OnFloatTypeParse}
<Type>           -> Bool {OnBoolTypeParse}
<Type>           -> Array LeftBracket <Type> RightBracket {CreateArrayType}

<Statement>      -> <Condition>
<Statement>      -> <Loop>
<Statement>      -> <Decl>
<Statement>      -> <Assign>
<Statement>      -> <Return>
<Statement>      -> <Composite>

<Condition>      -> If LeftParenthesis <Expression> RightParenthesis <Statement> {OnIfStatementParse} <OptionalElse>
<OptionalElse>   -> Else <Statement> {OnOptionalElseClauseParse}
<OptionalElse>   -> #Eps#

<Loop>           -> While LeftParenthesis <Expression> RightParenthesis <Statement> {OnWhileLoopParse}
<Decl>           -> Var Identifier {OnIdentifierParse} Colon <Type> Semicolon {OnVariableDeclarationParse}
<Assign>         -> Identifier {OnIdentifierParse} Assign <Expression> Semicolon {OnAssignStatementParse}
<Return>         -> Return <Expression> Semicolon {OnReturnStatementParse}

<Composite>      -> LeftCurly <StatementList> RightCurly {OnCompositeStatementParse}
<StatementList>  -> <Statement> {OnCompositeStatementPartParse} <StatementList>
<StatementList>  -> #Eps#

<Expression>     -> <AddSubExpr>
<AddSubExpr>     -> <MulDivExpr> <AddSubExprTail>
<AddSubExprTail> -> Plus <MulDivExpr> {OnBinaryPlusParse} <AddSubExprTail>
<AddSubExprTail> -> Minus <MulDivExpr> {OnBinaryMinusParse} <AddSubExprTail>
<AddSubExprTail> -> #Eps#
<MulDivExpr>     -> <AtomExpr> <MulDivExprTail>
<MulDivExprTail> -> Mul <AtomExpr> {OnBinaryMulParse} <MulDivExprTail>
<MulDivExprTail> -> Div <AtomExpr> {OnBinaryDivParse} <MulDivExprTail>
<MulDivExprTail> -> #Eps#
<AtomExpr>       -> Identifier {OnIdentifierParse}
<AtomExpr>       -> IntegerConstant {OnIntegerConstantParse}
<AtomExpr>       -> FloatConstant {OnFloatConstantParse}
<AtomExpr>       -> LeftParenthesis <Expression> RightParenthesis
<AtomExpr>       -> Minus <AtomExpr> {OnUnaryMinusParse}
