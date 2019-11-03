
const MongoClient = require('mongodb').MongoClient;
const url = "mongodb://admin:iot2019@localhost:27017/pump";

MongoClient.connect(url, { useUnifiedTopology: true }, function(err, db) {
  if (err) throw err;
  var dbo = db.db("pump");
  var myobj = { name: "Alexandre SAH", address: "30-6644 avenue Fielding H4V 1N3", tel:"(514)679-8529" };
  dbo.collection("values").insertOne(myobj, function(err, res) {
    if (err) throw err;
    console.log("1 document inserted");
    db.close();
  });
});
