<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    
    <title><?php print $head_title; ?></title>
    <?php print $head; ?>
    <?php print $scripts; ?>

    <link rel="stylesheet" href="/sites/all/modules/admin_menu/admin_menu.css">
  
    <!-- Latest compiled and minified CSS -->
    <link rel="stylesheet" href="//netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap.min.css">

    <!-- Optional theme -->
    <link rel="stylesheet" href="//netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap-theme.min.css">


    
</head>
<body class="<?php print $body_classes; ?> page-admin-watch">
  <?php print $content; ?>

  <div class="container-fluid">
    <?php if ($title): ?>
    <h1 class="title"></h1>
    <?php endif; ?>

    <div class="row">
        <div id="content-area">
            <div class="col-md-3">
                <div class="panel panel-default">
                  <div class="panel-heading">
                    <h3 class="panel-title">Active Users</h3>
                  </div>
                  <div class="panel-body" id="active-users" style="padding: 0">
                    Graph of new + active over 30 days
                  </div>
                </div>
            </div>
            <div class="col-md-3">
                <div class="panel panel-default">
                  <div class="panel-heading">
                    <h3 class="panel-title">New User Lifetime</h3>
                  </div>
                  <div class="panel-body">
                    Graph
                  </div>
                </div>
            </div>
            <div class="col-md-3">
                <div class="panel panel-default">
                  <div class="panel-heading">
                    <h3 class="panel-title">Products Sold (by Quantity)</h3>
                  </div>
                  <div class="panel-body" id="product-quantity" style="padding: 0">
                    Area Graph of all recent products sold
                  </div>
                </div>            
            </div>
            <div class="col-md-3">
                <div class="panel panel-default">
                  <div class="panel-heading">
                    <h3 class="panel-title">Products Sold (by Revenue)</h3>
                  </div>
                  <div class="panel-body" id="product-revenue">
                    Graph
                  </div>
                </div>
            </div>
        </div>        
    </div>
  </div>
  
    <!-- jQuery (necessary for Bootstrap's JavaScript plugins) -->
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.0/jquery.min.js"></script>
    
    <!-- Include all compiled plugins (below), or include individual files as needed -->
    <!-- Latest compiled and minified JavaScript -->
    <script src="//netdna.bootstrapcdn.com/bootstrap/3.1.1/js/bootstrap.min.js"></script>
    
    
    <script src="http://code.highcharts.com/highcharts.js"></script>
    <script src="http://code.highcharts.com/modules/exporting.js"></script>
    
  <?php print $closure; ?>
  
  <script type="text/javascript">
$(function () {

        // product-quantity
        console.debug('pq', product_quantity);
        if(product_quantity.length > 0) {
            var serdata = {};

            for(var i=0; i<product_quantity.length; i++) {
                var p = product_quantity[i];
                if(!serdata[p.title]) {
                    serdata[p.title] = {name: p.title, data: []};
                }
                var d = new Date(p.date);
                serdata[p.title].data[serdata[p.title].data.length] = [Date.UTC(d.getFullYear(), d.getMonth(), d.getDate()), parseInt(p.quantity)];
            }

            var ser = [];
            for(var i in serdata) {
                ser[ser.length] = serdata[i];
            }
            $('#product-quantity').highcharts({
                chart: {
                    type: 'column'
                },
                title: {
                    text: ''
                },
                xAxis: {
                    type: 'datetime',
                    title: {
                        text: ''
                    }
                },
                yAxis: {
                    title: {
                        text: ''
                    }
                },
                tooltip: {
                    shared: true
                },
                plotOptions: {
                    area: {
                        stacking: 'normal',
                    }
                },
                series: ser,
                credits: {
                    enabled: false
                }
            });
        }
                
        // product-revenue
        $('#product-revenue').highcharts({
            chart: {
                type: 'area'
            },
            title: {
                text: 'Historic and Estimated Worldwide Population Growth by Region'
            },
            subtitle: {
                text: 'Source: Wikipedia.org'
            },
            xAxis: {
                categories: ['1750', '1800', '1850', '1900', '1950', '1999', '2050'],
                tickmarkPlacement: 'on',
                title: {
                    enabled: false
                }
            },
            yAxis: {
                title: {
                    text: 'Billions'
                },
                labels: {
                    formatter: function() {
                        return this.value / 1000;
                    }
                }
            },
            tooltip: {
                shared: true,
                valueSuffix: ' millions'
            },
            plotOptions: {
                area: {
                    stacking: 'normal',
                    lineColor: '#666666',
                    lineWidth: 1,
                    marker: {
                        lineWidth: 1,
                        lineColor: '#666666'
                    }
                }
            },
            series: [{
                name: 'Asia',
                data: [502, 635, 809, 947, 1402, 3634, 5268]
            }, {
                name: 'Africa',
                data: [106, 107, 111, 133, 221, 767, 1766]
            }, {
                name: 'Europe',
                data: [163, 203, 276, 408, 547, 729, 628]
            }, {
                name: 'America',
                data: [18, 31, 54, 156, 339, 818, 1201]
            }, {
                name: 'Oceania',
                data: [2, 2, 2, 6, 13, 30, 46]
            }],
            credits: {
                enabled: false
            }
        });

        console.debug('au', users);
        if(users.length > 0) {
            var serdata = {active_users:{name:'Active', data:[]}, new_users:{name:'New', data:[]}};
            for(var i=0; i<users.length; i++) {
                var p = users[i];
                var d = new Date(p.dday);
                serdata.active_users.data = [Date.UTC(d.getFullYear(), d.getMonth(), d.getDate()), parseInt(p.active_users)];
                serdata.new_users.data = [Date.UTC(d.getFullYear(), d.getMonth(), d.getDate()), parseInt(p.new_users)];
            }

            var ser = [];
            for(var i in serdata) {
                ser[ser.length] = serdata[i];
            }
            // active users
            $('#active-users').highcharts({
                chart: {
                    type: 'area'
                },
                title: {
                    text: 'New vs. Active Users'
                },
                xAxis: {
                    type: 'datetime',
                    title: {
                        text: ''
                    }
                },
                yAxis: {
                    title: {
                        text: ''
                    }
                },
                tooltip: {
                    shared: true
                },
                plotOptions: {
                    area: {
                        stacking: 'normal'
                    }
                },
                series: ser,
                credits: {
                    enabled: false
                }
            });
        }

    });
</script>


</body>
</html>
