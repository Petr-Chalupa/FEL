module Hw03 where


type Symbol = String
data Expr = Var Symbol
          | App Expr Expr
          | Lambda Symbol Expr deriving Eq


instance Show Expr where
  show (Var x) = x
  show (App ex1 ex2) = "(" ++ (show ex1) ++ " " ++ (show ex2) ++ ")"
  show (Lambda x ex) = "(\\" ++ x ++ "." ++ (show ex) ++ ")"


genFreshVar :: [Symbol] -> Symbol
genFreshVar usedVars = head (filter (`notElem` usedVars) (map (\n -> "a" ++ show n) [0..]))


findFreeVars :: Expr -> [Symbol]
findFreeVars expr =
  case expr of
    Var x -> [x]
    App e1 e2 -> (findFreeVars e1) ++ (findFreeVars e2)
    Lambda x body -> filter (/= x) (findFreeVars body)


alphaConvert :: Expr -> Symbol -> Symbol -> Expr
alphaConvert expr var sub =
  case expr of
    Var v ->
      if v == var
        then Var sub
        else Var v
    App e1 e2 ->
      App (alphaConvert e1 var sub) (alphaConvert e2 var sub)
    Lambda v body ->
      if v == var
        then Lambda sub (alphaConvert body var sub)
        else Lambda v (if v == sub then body else (alphaConvert body var sub))


substitute :: Expr -> Symbol -> Expr -> Expr
substitute expr var e =
  case expr of
    Var v ->
      if v == var
        then e
        else Var v
    App e1 e2 ->
      App (substitute e1 var e) (substitute e2 var e)
    Lambda v body ->
      if v == var
        then Lambda v body
        else if v `elem` (findFreeVars e)
          then
            let usedVars = (findFreeVars body) ++ (findFreeVars e) ++ [v, var]
                newVar = genFreshVar usedVars
                alphaConvertedBody = alphaConvert body v newVar
            in Lambda newVar (substitute alphaConvertedBody var e)
          else
            Lambda v (substitute body var e)


betaReduce :: Expr -> Expr -> Expr
betaReduce expr e =
  case expr of
    Lambda v body -> substitute body v e
    _ -> expr
      

findLE :: Expr -> Maybe (Expr, Expr)
findLE expr =
  case expr of
    Var v -> Nothing
    App e1 e2 ->
      case findLE e1 of
        Just leftmostApp -> Just leftmostApp
        Nothing -> 
          case findLE e2 of
            Just leftmostApp -> Just leftmostApp
            Nothing -> 
              case e1 of
                Lambda v body -> Just (e1, e2)
                _ -> Nothing           
    Lambda v body -> findLE body


eval :: Expr -> Expr
eval expr =
  case expr of
    Var x -> Var x
    Lambda v body -> Lambda v (eval body)
    App e1 e2 ->
      let evE1 = eval e1
          evE2 = eval e2
      in case evE1 of
        Lambda v body ->
          let reduced = betaReduce evE1 evE2
          in if reduced == App evE1 evE2 
            then reduced
            else eval reduced
        _ -> App evE1 evE2
        