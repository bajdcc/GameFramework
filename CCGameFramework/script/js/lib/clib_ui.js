UI.prototype.render = function() {
    this.render_internal();
    this.map(x => x.render());
};
UI.root = new UI({
    type: 'root',
    left: 0,
    top: 0,
    width: 0,
    height: 0
});
sys.send_signal = (function() {
    const map_signal = {
        render: function() {
            UI.root.render();
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