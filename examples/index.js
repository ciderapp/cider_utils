const { parseFile } = require("../index.node");
const fs = require('fs');
 
// json data
let jsonData = parseFile("e:\\VA\\Music\\Osu music pack completed\\besssst.mp3");
let u = jsonData
// replace artwork with artwork size in console.log
u["artwork"] = (u["artwork"] ?? '').length
console.log(u) 
// stringify JSON Object
let jsonContent = JSON.stringify(jsonData);

fs.writeFile("output.json", jsonContent, 'utf8', function (err) {
    if (err) {
        console.log("An error occured while writing JSON Object to File.");
        return console.log(err);
    }
 
    console.log("JSON file has been saved.");
});

