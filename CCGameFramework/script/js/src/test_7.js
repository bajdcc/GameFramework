setTimeout(function() {
    console.log('Please input JavaScript code: ');
}, 4000);
setTimeout(function() {
    sys.event.on('input', function(text) {
        if (text) {
            try {
                console.log(eval(text));
            } catch (e) {
                console.error(e);
            }
        }
        sys.config({
            'input': {
                'enable': true
            }
        });
    });
    sys.config({
        'input': {
            'enable': true
        }
    });
}, 100);