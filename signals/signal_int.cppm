export module signals.signal_int;

export import signals.base;

// NOTE: the module is named signals.signal_int, not signals.int — `int`
// is a keyword and cannot appear as a module-name component.
export using SignalInt = Signal<int>;
