UI.prototype.render = function(left, top) {
    this.render_internal(left, top);
    if (this.layout !== 'absolute') {
        this.map(x => x.render(left, top));
    } else {
        this.map(x => x.render(this.left + left, this.top + top));
    }
};
UI.root = new UI({
    type: 'root',
    left: 0,
    top: 0,
    width: 0,
    height: 0
});
sys.send_signal = (function() {
    var map_signal = {
        render: function() {
            UI.root.render(UI.root.left, UI.root.top);
        },
        resize: function() {
            UI.root.width = sys.get_config('screen/width');
            UI.root.height = sys.get_config('screen/height');
        }
    };
    return function(n) {
        return map_signal[n]();
    }
})();
sys.send_signal("resize");
return;