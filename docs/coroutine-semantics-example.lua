function inorder(node)
    if node then
        inorder(node.left)
        coroutine.yield(node.key)
        inorder(node.right)
    end
end

function inorder_iterator(tree)
    return coroutine.wrap(function() inorder(tree) return nil end)
end

example_tree = {key=3, left={key=1, right={key=2}}, right={key=8, left={key=5, left={key=4}, right={key=7, left={key=6}}}, right={key=9}}}
iterator = inorder_iterator(example_tree)

print(iterator())
-- 1
print(iterator())
-- 2
print(iterator())
-- 3
print(iterator())
-- 4
print(iterator())
-- 5
print(iterator())
-- 6
print(iterator())
-- 7
print(iterator())
-- 8
print(iterator())
-- 9
print(iterator())
-- nil
print(iterator())
-- stdin:1: cannot resume dead coroutine
-- stack traceback:
--         [C]: in function 'iterator'
--         stdin:1: in main chunk
--         [C]: in ?
