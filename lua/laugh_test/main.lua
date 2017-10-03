
local Foo = require 'foo'

local Bar = new.class(Foo)

function Bar:init()
    Foo.init(self, 'BAR BAR BAR BAR')
end

return function (name, argv)
    print(name)
    for i, arg in ipairs(argv) do
        print(i, arg)
    end

    print()
    print(new(Bar)():speak())
end
