function Event(evt) {
    this.listener = {};
    if (evt) {
        for (var i in evt) {
            this.on(i, evt[i]);
        }
    }
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
Event.prototype.emit = function(type, obj, ...args) {
    if (this.listener[type])
        this.listener[type].forEach(x => x.apply(obj, args));
    return this;
};
sys.event = new Event();
return;