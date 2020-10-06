# Change Log

## 2020.10.6

+ Remove extra `undefined` argument when calling a `LuaObject` function (see [LuaObject.cpp#L123-L133](duktape/src/LuaObject.cpp#L123-L133))

  If you want to pass `nil` (in lua) to it, please use `null` (in js).

  This feature is useful when writing wrapper libraries.

+ When passing an `Array` as an argument to `LuaObject` function, it will be turned to an array-like table which starts its index at `1`. (see [binder.cpp#L76-L86](duktape/src/binder.cpp#L76-L86))

  Actually, you can not pass an `Array` without this fix, because `xxx["1"]` is the same as `xxx[1]` in JavaScript, but not in lua.

+ Support bind a js function to lua. (see [CallbackBinder.h](duktape/src/CallbackBinder.h) and [CallbackBinder.cpp](duktape/src/CallbackBinder.cpp))

  This feature is mainly designed for `go.animate`'s callback function.

## 2020.10.4

first ver
