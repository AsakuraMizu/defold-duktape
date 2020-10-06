module.exports = {
    init: function (self) {
        lua.print.apply(null, [0, "Hello, Defold in JavaScript!"]);
        lua.msg.post(0, ".", "example message", [1, 2, 3]);
        self.what = "message";
        self.f(0, "lua function");
        self.f = function(x) {
            lua.print(0, "bind a " + x);
        };
    },
    on_message: function(self, message_id, message, sender) {
        // Template literals are not supported in ES5!
        // lua.print(0, `received ${self.what}:`);
        lua.print(0, "received " + self.what + ":", message_id);

        if (lua.hash_to_hex(1, message_id) == lua.hash_to_hex(1, lua.hash(1, "example message"))) {
            lua.print(0, "it's sad that you can only compare two hash via \"hash_to_hex\":(");
        }
    },
};
