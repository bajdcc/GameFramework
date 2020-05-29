(function () {
    var i = 0;
    var t1, t2;
    t1 = setTimeout(function a(n) {
        if (++i > 4) return;
        console.log(n, t1, i);
        setTimeout(a, 1000, n);
    }, 1000, "setTimeout:  ");
    t2 = setInterval(function a(n) {
        if (++i > 4) return clearInterval(t2);
        console.log(n, t2, i);
    }, 1000, "setInterval: ");
})();