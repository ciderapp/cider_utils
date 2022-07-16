const ciderutils = require("./build/Release/ciderutils.node");

module.exports = {
    parseFile: ciderutils.parseFile,
    recursiveFolderSearch: ciderutils.recursiveFolderSearch
};
