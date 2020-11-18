module.exports = {
    init: function (self) {
        // lua.print.apply(null, ["Hello, Defold in JavaScript!"]);
        lua.msg.post(".", "example message");
        self.what = "message";
        self.f("lua function");
        self.f = function(x) {
            lua.print("bind a " + x);
        };
        lua.print(lua.go.get_position());
    },
    on_message: function(self, message_id, message, sender) {
        // Template literals are not supported in ES5!
        // lua.print(`received ${self.what}:`);
        lua.print("received " + self.what + ":", message_id);

        if (lua.hash_to_hex(message_id) == lua.hash_to_hex(lua.hash("example message"))) {
            lua.print("it's sad that you can only compare two hash via \"hash_to_hex\":(");
        }
    },
};
