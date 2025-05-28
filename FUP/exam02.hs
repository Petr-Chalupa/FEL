module SpanningTree (minSpanningTree, Graph (..), Edge (..)) where
import Data.List -- for sortOn
 
data Edge a b = Edge { u :: a,
                       v :: a,
                       weight :: b } deriving (Eq,Show)
 
data Graph a b = Graph { nodes :: [a],
                         edges :: [Edge a b] } deriving Show
 
 
----------
 
 
reverseEdge :: Edge a b -> Edge a b
reverseEdge e = Edge{ u = v e, v = u e, weight = weight e }
 
 
reverseEdges :: Graph a b -> Graph a b
reverseEdges gr = Graph{ nodes = nodes gr, edges = edges gr ++ ies }
    where
        ies = map reverseEdge (edges gr)
 
 
jarnik :: (Ord b, Eq a) => [Edge a b] -> [a] -> [a] -> [Edge a b] -> [Edge a b]
jarnik _ _ [] tree = tree
jarnik es covered uncovered tree = jarnik es newCovered newUncovered newTree
    where
        filteredEs   = [e | e <- es, u e `elem` covered, v e `elem` uncovered]
        sortedEs     = sortOn weight filteredEs
        nextEdge     = head sortedEs
        nextNode     = v nextEdge
        newCovered   = nextNode : covered
        newUncovered = filter (/= nextNode) uncovered
        newTree      = nextEdge : tree
 
 
minSpanningTree :: (Ord b, Eq a) => Graph a b -> [Edge a b]
minSpanningTree gr = jarnik (edges ngr) firstNode restNodes []
    where
        ngr       = reverseEdges gr
        firstNode = [head $ nodes ngr]
        restNodes = tail $ nodes ngr