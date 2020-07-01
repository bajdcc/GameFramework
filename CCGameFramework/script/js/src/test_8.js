UI.root.push(
    new UI({
        type: 'label',
        color: '#ffffff',
        content: 'Hello world!',
        left: 10,
        top: 110,
        width: 100,
        height: 100,
        align: 'center',
        valign: 'center',
        font: {
            family: 'KaiTi',
            size: 16
        }
    })
);
UI.root.push(
    new UI({
        type: 'rect',
        color: '#ff0000',
        left: 10,
        top: 50,
        width: 100,
        height: 100,
    })
);