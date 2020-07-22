(function() {
    var top = 350,
        left = 20,
        size = 6,
        w = 30,
        h = 30,
        gw = 5,
        gh = 5,
        radius = 5;

    function trans(blks) {
        return blks.map(blk => new UI({
            type: 'round',
            radius: radius,
            _left: blk.left,
            _top: blk.top,
            _right: blk.right,
            _bottom: blk.bottom,
            _target: blk.target,
            _calc: function() {
                this.color = this._target ? '#ff0000' : 'rgb(226,187,80)';
                this.left = left + (blk.left - 1) * (w + gw);
                this.top = top + (blk.top - 1) * (h + gh);
                this.width = (blk.right - blk.left + 1) * w + (blk.right - blk.left) * gw;
                this.height = (blk.bottom - blk.top + 1) * h + (blk.bottom - blk.top) * gh;
            },
            event: new Event({
                'hit': function(t, x, y) {
                    console.log(this.type, this.left, this.top, this.color, t, x, y);
                }
            })
        }));
    }
    var bg = new UI({
        type: 'rect',
        color: 'rgb(0,30,30)',
        left: left - gw,
        top: top - gw,
        width: size * (w + gw) + gw,
        height: size * (h + gh) + gh
    });
    UI.root.push(bg);
    var blocks = {
        type: 'Klotski-1',
        size: size,
        blocks: [
            { left: 1, top: 1, right: 1, bottom: 2 },
            { left: 2, top: 1, right: 3, bottom: 1 },
            { left: 4, top: 1, right: 5, bottom: 1 },
            { left: 3, top: 2, right: 3, bottom: 3 },
            { left: 4, top: 2, right: 4, bottom: 4 },
            { left: 5, top: 2, right: 6, bottom: 2 },
            { left: 1, top: 3, right: 2, bottom: 3, target: true },
            { left: 2, top: 4, right: 3, bottom: 4 },
            { left: 2, top: 5, right: 4, bottom: 5 },
            { left: 5, top: 3, right: 5, bottom: 5 },
            { left: 1, top: 6, right: 2, bottom: 6 },
        ]
    };
    var blocks_gui = trans(blocks.blocks);
    blocks_gui.forEach(b => b._calc());
    blocks_gui.forEach(b => UI.root.push(b));
    var solve = sys.helper('puzzle', {
        type: 'Klotski-1',
        size: 6,
        blocks
    });
    if (solve instanceof String)
        console.error(solve);
    else {
        console.log(JSON.stringify(solve));
        var time = 0;
        setTimeout(function draw() {
            if (time++ > 10)
                return;
            setTimeout(draw, 1000);
        }, 1000);
    }
})();