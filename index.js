const ciderutils = require("path").resolve(__dirname, "./build/Release/ciderutils.node");;

module.exports = {
    parseFile: ciderutils.parseFile,
    recursiveFolderSearch: ciderutils.recursiveFolderSearch
};
