module Hw4 where


import Control.Applicative
import Data.Char
import Parser
import Hw3


parseVar :: Parser Expr
parseVar = do
    var <- some alphaNum
    return (Var var)


parseLambda :: Parser Expr
parseLambda = do
    string "(\\"
    var <- some alphaNum
    char '.'
    expr <- parseExpr
    char ')'
    return $ Lambda var expr


parseApp :: Parser Expr
parseApp = do
    char '('
    ex1 <- parseExpr
    sep
    ex2 <- parseExpr
    char ')'
    return $ App ex1 ex2


parseExpr :: Parser Expr
parseExpr = parseLambda <|> parseApp <|> parseVar


parseDef :: Parser (Symbol, Expr)
parseDef = do
    var <- some alphaNum
    sep
    string ":="
    sep
    expr <- parseExpr
    return (var, expr)


replace :: [(Symbol, Expr)] -> Expr -> Expr
replace env expr =
    case expr of
        Var x -> 
            case lookup x env of
                Just e -> e
                Nothing -> Var x
        App e1 e2 -> App (replace env e1) (replace env e2)
        Lambda x e -> Lambda x (replace env e)


replaceAll :: [(Symbol, Expr)] -> Expr -> Expr
replaceAll env e =
    let e' = replace env e
    in if e' == e then e else replaceAll env e'


parseProgram :: Parser Expr
parseProgram = do
    defs <- many $ do
        def <- parseDef
        sep
        return def
    expr <- parseExpr
    return $ replaceAll defs expr


readPrg :: String -> Maybe Expr
readPrg prg = case parse parseProgram prg of
    Just (ex, rest) | all isSpace rest -> Just ex
    _ -> Nothing