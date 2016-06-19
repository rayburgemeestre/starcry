// The raw data to observe
var stats = [];

// A resusable polygon graph component
Vue.component('polygraph', {
    props: ['stats'],
    template: '#polygraph-template',
    replace: true,
    computed: {
        // a computed property for the polygon's points
        points: function () {
            var total = this.stats.length
            return this.stats.map(function (stat, i) {
                var point = valueToPoint(stat.value, i, total)
                return point.x + ',' + point.y
            }).join(' ')
        }
    },
    components: {
        // a sub component for the labels
        'axis-label': {
            props: {
                stat: Object,
                index: Number,
                total: Number
            },
            template: '#axis-label-template',
            replace: true,
            computed: {
                point: function () {
                    return valueToPoint(
                        +this.stat.value + 10,
                        this.index,
                        this.total
                    )
                }
            }
        }
    }
})

// math helper...
function valueToPoint (value, index, total) {
    var x     = 0
    var y     = -value * 0.8
    var angle = Math.PI * 2 / total * index
    var cos   = Math.cos(angle)
    var sin   = Math.sin(angle)
    var tx    = x * cos - y * sin + 100
    var ty    = x * sin + y * cos + 100
    return {
        x: tx,
        y: ty
    }
}

// bootstrap the demo
var v = new Vue({
    el: '#demo',
    data: {
        newLabel: '',
        stats: stats
    },
    methods: {
        add: function (e) {
            e.preventDefault()
            if (!this.newLabel) return
            this.stats.push({
                label: this.newLabel,
                value: 100
            })
            this.newLabel = ''
        },
        remove: function (stat) {
            if (this.stats.length > 3) {
                this.stats.$remove(stat)
            } else {
                alert('Can\'t delete more!')
            }
        }
    }
});

$.ajax({
    dataType: "json",
    url: '/api/test',
    data: {},
    success: function (data) {
        console.log("success")
        console.log(data['data'])
        v.stats = data['data'];
    }
});