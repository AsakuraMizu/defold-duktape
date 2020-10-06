# JavaScript language binding for Defold game engine

This library allows writing [JavaScript](https://www.javascript.com/) code for [Defold](https://defold.com/) game engine.

[Duktape](https://duktape.org/) is used as the JavaScript engine.

## Features

+ [ECMAScript E5/E5.1](http://www.ecma-international.org/ecma-262/5.1/http://www.ecma-international.org/ecma-262/5.1/), partial support for [ECMAScript 2015 (E6)](http://www.ecma-international.org/ecma-262/6.0/index.html) and [ECMAScript 2016 (E7)](http://www.ecma-international.org/ecma-262/7.0/index.html)
+ CommonJS `require` support
+ All Defold api avaliable (and native extension)
+ Extension support via `lua.require`

## Installation

You can use defold-duktape in your own project by adding this project as a [Defold library dependency](https://defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

https://github.com/AsakuraMizu/defold-duktape/archive/master.zip

Or point to the ZIP file of a [specific release](https://github.com/AsakuraMizu/defold-duktape/releases).

## Quick start

1. Create a `example.js`
   ```js
   module.exports = {
       init: function (self) {
           lua.print(0, "Hello, Defold in JavaScript!");
           lua.msg.post(0, ".", "example message", [1, 2, 3]);
       },
       on_message: function(self, message_id, message, sender) {
           lua.print(0, message_id);
           lua.pprint(0, message);
       },
   };
   ```
2. Create a `example.script`
   ```lua
   local script = duktape.load("res/example.js")

   function init(self)
       script:init(self)
   end

   function final(self)
       script:final(self)
   end

   function update(self, dt)
       script:update(self, dt)
   end

   function on_message(self, message_id, message, sender)
       script:on_message(self, message_id, message, sender)
   end

   function on_input(self, action_id, action)
       script:on_input(self, action_id, action)
   end

   function on_reload(self)
       script:on_reload(self)
   end
   ```
3. Add `example.js` to your project's [Custom Resources](https://defold.com/manuals/project-settings/#custom-resources)
4. Create a game object and add `example.script` to it
5. Enjoy!

## API

### Script URI

Script uri is a string to locate a javascript file. It can be:
+ `res/path/to/script.js`: load the script from your project's [Custom Resources](https://defold.com/manuals/project-settings/#custom-resources)

This is designed for `duktape.load`, but you can also use with `require` in javascript.

### Lua API

#### duktape.load(uri)
Load a script

**PARAMETERS**
+ `uri`: the uri of the script

**RETURNS**
+ `script`: the loaded script

#### Script

The loaded script instance. Can be created only via `duktape.load`.

It has the following methods:

```lua
script:init(self)
script:final(self)
script:update(self, dt)
script:on_message(self, message_id, message, sender)
script:on_input(self, action_id, action)
script:on_reload(self)
```

These functions will call the exported javascript function. If not found (not exported), do nothing.

### JavaScript API

#### Duktape built-ins

See https://duktape.org/guide.html#duktapebuiltins

#### lua

The lua global table. It is a `LuaObject` internally.

#### LuaObject

An object type, which is actually [ES6 Proxy](http://es6-features.org/#Proxying), to wrap lua table, userdata or function.

Assuming `lo` is a `LuaObject`:

**get** When you write `lo.xxx` or `lo["xxx"]` in javascript, it calls `lua_gettable`, which is equivalent to `lo["xxx"]` in lua. After this you will get a `undefined`, `boolean`, `number`, `string`, or a new `LuaObject`.

**set** When you write `lo.xxx = yyy` or `lo["xxx"] = yyy` in javascript, it calls `lua_settable`, which is equivalent to `lo["xxx"]` in lua.

**delete** When you write `delete lo.xxx` or `delete lo["xxx"]` in javascript, it is equivalent to `lo.xxx = undefined` or `lo["xxx"] = undefined`.

**call** Because lua supports returning multiple results but javascript does not, you have to specify the number of results when you call it. The first parameter you pass in will be used as the number. So when you write `lo(n, ...args)`, it is equivalent to `res1, res2, ..., resn = lo(...args)` in lua. If `n` equals to 0, `undefined` will be returned; if `n` equals to 1, `res1` will be returned; otherwise, an array contains all `n` results will be returned. If `n` is not equal to the actual number of results, extra ones will be dropped and missing ones will padded with undefined as necessary.

## Todo

+ Examples
+ Hot reload
+ TypeScript support
+ Wrapped api (there is only a `lua` currently)
+ Debug in vscode (see https://github.com/harold-b/vscode-duktape-debug)
+ More script uri protocol (e.g. `file`, `user`, `http`)

## About

+ I have tried to use [QuickJS](https://bellard.org/quickjs) but failed (https://github.com/AsakuraMizu/defold-quickjs)
+ Issues and PRs are welcomed.

### Known issues

+ If you pass a non-exist uri to `duktape.load`, no error will be logged and it will work as an empty script.
+ Don't `"use strict"`.
