# Change Log

## 2020.10.6

+ Remove extra `undefined` argument when calling a `LuaObject` function (see [LuaObject.cpp#L135-L145](duktape/src/LuaObject.cpp#L135-L145))

  If you want to pass `nil` (in lua) to it, please use `null` (in js).

  This feature is useful when writing wrapper libraries.

+ When passing an `Array` as an argument to `LuaObject` function, it will be turned to an array-like table which starts its index at `1`. (see [binder.cpp#L80-L90](duktape/src/binder.cpp#L80-L90))

  Actually, you can not pass an `Array` without this fix, because `xxx["1"]` is the same as `xxx[1]` in JavaScript, but not in lua.

+ Support bind a js function to lua. (see [CallbackBinder.h](duktape/src/CallbackBinder.h) and [CallbackBinder.cpp](duktape/src/CallbackBinder.cpp))

  This feature is mainly designed for `go.animate`'s callback function.

+ Fix `LuaObject(handler).ownKeys`. Note that you have to use `Reflect.ownKeys` instead of `Object.keys`.

+ Advanced `LuaObject(handler).get`, which makes it possible to use `LuaObject` as a normal Function.

+ Fix error stack logging.

## 2020.10.4

First version.
