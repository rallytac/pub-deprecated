{
  "targets": 
  [
    {
      "target_name": "engage",
      "sources": [ "engage.cpp" ],
      "include_dirs": [ 
        "../../api/c/include/",    
        "<!(node -e \"require('nan')\")",
        "<!(node -e \"require('node-gyp')\")",
      ],

       "link_settings": {
         "libraries": ["-lengage-static"],
       },

      "conditions": [
        ["OS==\"mac\"", {          
          "library_dirs": ['<(module_root_dir)/../../bin/<!(node engage_yarn_tools.js -maxversion:<(module_root_dir)/../../bin)/darwin']
        }],
        ["OS==\"linux\"", {
          "library_dirs": ['<(module_root_dir)/../../bin/<!(node engage_yarn_tools.js -maxversion:<(module_root_dir)/../../bin)/centos7']
        }],
        ["OS==\"win\"", {
          "library_dirs": ['<(module_root_dir)/../../bin/<!(node engage_yarn_tools.js -maxversion:<(module_root_dir)/../../bin)/win/ia32']
        }],
      ]
    }
  ]
}
