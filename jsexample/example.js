module.exports = {
    init: function (self) {
        lua.print(0, "Hello, Defold in JavaScript!");
        lua.msg.post(0, ".", lua.hash(1, "example message"));
    },
    on_message: function(self, message_id, message, sender) {
        lua.print(0, message_id);
    },
};