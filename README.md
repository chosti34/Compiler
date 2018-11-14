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
<Type>           -> Integer {CreateIntegerType}
<Type>           -> Float {CreateFloatType}
<Type>           -> Bool {CreateBoolType}
<Type>           -> Array LeftBracket <Type> RightBracket {CreateArrayType}
<Statement>      -> <Condition>
<Statement>      -> <Loop>
<Statement>      -> <Decl>
<Statement>      -> <Assign>
<Statement>      -> <Return>
<Statement>      -> <Composite>
<Condition>      -> If LeftParenthesis <Expression> RightParenthesis <Statement> {CreateIfStatement} <OptionalElse>
<OptionalElse>   -> Else <Statement> {AddOptionalElseClause}
<OptionalElse>   -> #Eps#
<Loop>           -> While LeftParenthesis <Expression> RightParenthesis <Statement> {CreateWhileStatement}
<Decl>           -> Var Identifier {CreateIdentifierNode} Colon <Type> Semicolon {CreateVariableDeclStatement}
<Assign>         -> Identifier {CreateIdentifierNode} Assign <Expression> Semicolon {CreateAssignStatement}
<Return>         -> Return <Expression> Semicolon {CreateReturnStatement}
<Composite>      -> LeftCurly <StatementList> RightCurly {CreateCompositeStatement}
<StatementList>  -> <Statement> {AddStatementToComposite} <StatementList>
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
