map_ :: (a -> b) -> [a] -> [b]
map_ fn (l:ls) = fn l : map_ fn ls
