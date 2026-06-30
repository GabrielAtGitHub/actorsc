export module signals.signal_bool;

export import signals.base;

// NOTE: the module is named signals.signal_bool, not signals.bool — `bool`
// is a keyword and cannot appear as a module-name component.
export using SignalBool = Signal<bool>;
