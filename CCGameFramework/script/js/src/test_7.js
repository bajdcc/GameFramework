setTimeout(function() {
    sys.event.on('input', function(text) {
        console.log(eval(text));
    });
    sys.event.emit('input', '"Please input JavaScript code: "');
}, 4000);