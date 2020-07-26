UI.prototype.render = function(n) {
    if (n === 1)
        this.render_internal(n);
    this.map(x => x.render(n));
    if (n === 2)
        this.render_internal(n);
};
UI.root = new UI({
    type: 'root',
    left: 0,
    top: 0,
    width: 0,
    height: 0
});
UI.get_layout = (function() {
    const map_layout = {
        linear: function() {
            if (!this.orientation)
                this.orientation = 'vertical';
            if (this.orientation == 'horizontal') {
                var w = this.map(x => x.weight || 0);
                var all = w.reduce((a, b) => a + b, 0);
                if (all === 0)
                    return;
                all = 1 / all;
                var sum = 0;
                for (var i in this) {
                    var o = this[i];
                    o.left = this.left + sum;
                    o.width = Math.ceil(this.width * w[i] * all);
                    sum += o.width;
                    o.top = this.top;
                    o.height = this.height;
                    if (o.event)
                        o.event.emit('resize', o);
                }
            } else if (this.orientation == 'vertical') {

            }
        }
    };
    return function(t) {
        return map_layout[t];
    }
})();
sys.send_signal = (function() {
    const map_signal = {
        render: function() {
            UI.root.render(1);
            UI.root.render(2);
        },
        resize: function() {
            UI.root.width = sys.get_config('screen/width');
            UI.root.height = sys.get_config('screen/height');
        },
        hit: function() {
            var obj = sys.get_config('hit/obj');
            var type = sys.get_config('hit/type');
            var x = sys.get_config('hit/x');
            var y = sys.get_config('hit/y');
            if (obj && obj.event) {
                obj.event.emit('hit', obj, type, x, y);
            }
        }
    };
    return function(n) {
        return map_signal[n]();
    }
})();
sys.send_signal("resize");
return;