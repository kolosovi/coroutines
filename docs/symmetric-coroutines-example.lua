coro = {}
coro.main = function() end
coro.current = coro.main

-- function to create a new coroutine
function coro.create(f)
    local co = function(val)
        f(val)
        error("coroutine ended")
    end
    return coroutine.wrap(co)
end

-- function to transfer control to a coroutine
function coro.transfer(co, val)
    if coro.current ~= coro.main then
        return coroutine.yield(co, val)
    end
    -- dispatching loop
    while true do
        coro.current = co
        if co == coro.main then
            return val
        end
        co, val = co(val)
    end
end

function print_steps(first)
    local num_iters = 5
    local step = 3
    local current = first
    while num_iters > 0 do
        print(first, current)
        current = current + step
        num_iters = num_iters - 1
        local new_first = (first % 3) + 1
        coro.transfer(queue[new_first], new_first)
    end
end

queue = {coro.create(print_steps), coro.create(print_steps), coro.create(print_steps)}
coro.transfer(queue[1], 1)
