function Event() {
    this.listener = {};
}
Event.prototype.on = function(type, f) {
    this.listener[type] = this.listener[type] || [];
    this.listener[type].push(f);
    return this;
};
Event.prototype.once = function(type, f) {
    var wrap = (...args) => {
        f.apply(this, args);
        this.off(event, wrap);
    };
    this.on(event, wrap);
    return this;
};
Event.prototype.off = function(type, f) {
    if (this.listener[type])
        this.listener[type] = this.listener[type].filter(x => x !== f);
    return this;
};
Event.prototype.emit = function(type, ...args) {
    if (this.listener[type])
        this.listener[type].forEach(x => x.apply(this, args));
    return this;
};
sys.event = new Event();
return;