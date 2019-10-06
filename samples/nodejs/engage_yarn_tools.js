//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

var fs = require('fs');

//--------------------------------------------------------
for( var x = 2; x < process.argv.length; x++ )
{
    if(process.argv[x].startsWith("-maxversion:"))
    {
        var maxVersion = get_max_bin_version(process.argv[2].substring(12));

        if( maxVersion == undefined ) 
        {
            console.error("ERROR: Cannot determine latest binary version");
        }
        else
        {
            console.log(maxVersion);
        }

        break;
    }
    else
    {
        console.error("ERROR: unrecognized option '" + process.argv[x] + "'");
        show_syntax();
    }
}


//--------------------------------------------------------
function show_syntax()
{
    console.log("usage: engage_yt [-maxversion:<bin_directory>]")
}


//--------------------------------------------------------
function get_max_bin_version(path)
{
    var maxVerPath = undefined;

    try
    {
        var maxVerInt = 0;
        var items = fs.readdirSync(path)

        for( var x = 0; x < items.length; x++ ) 
        {
            var stats = fs.statSync(path + "/" + items[x]);

            if(stats.isDirectory()) 
            {
                // Example: 1.83.8833
                var numArray = items[x].split(".");

                if( numArray.length == 3 )
                {
                    var major = (parseInt(numArray[0]) * 1000000000);
                    var minor = (parseInt(numArray[1]) * 100000);
                    var build = parseInt(numArray[2]);

                    var expanded = (major + minor + build);

                    if(expanded > maxVerInt) 
                    {
                        maxVerInt = expanded;
                        maxVerPath = items[x];
                    }    
                }
            }
        }
    }
    catch
    {
        console.error("ERROR: error while querying directory");
        maxVerPath = undefined;
    }

    return maxVerPath;
}
