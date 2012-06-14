<?php 

$db = NULL;  

function open_db() {
  $db = new PDO("nuodb:database=test@localhost;schema=Hockey", "dba", "goalie") or die;
  return $db;
}

function query_with_row_count($sql) {
  $row_count = 0;
  try {  
    $db = open_db();
    foreach ($db->query($sql) as $row) {
      $row_count++;
    }
    $db = NULL;
  } catch(PDOException $e) {  
    echo $e->getMessage();  
  }
  $db = NULL;  
  return $row_count;
}

// select test1
$sql = "select * from hockey where NUMBER<12";
$row_count = query_with_row_count($sql);
if ($row_count != 2) {
   print("FAILED: $sql\n");
}

// select test2
$sql = "select * from hockey";
$row_count = query_with_row_count($sql);
if ($row_count != 25) {
   print("FAILED: $sql\n");
}


// insert test1
try {  
  $db = open_db();
  $sql = "INSERT INTO hockey(NUMBER, NAME, POSITION, TEAM) VALUES('99', 'Mickey Mouse', 'Center', 'Disney')";
  $count = $db->exec($sql);
  $db = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  

// select test3
$sql = "select * from hockey";
$row_count = query_with_row_count($sql);
if ($row_count != 26) {
   print("FAILED: $sql\n");
}

// delete test1
try {  
  $db = open_db();
  $sql = "DELETE FROM hockey WHERE TEAM='Disney'";
  $count = $db->exec($sql);
  $db = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  

// select test4
$sql = "select * from hockey";
$row_count = query_with_row_count($sql);
if ($row_count != 25) {
   print("FAILED: $sql\n");
}
$db = NULL;

// select test5 - query and fetch with FETCH_ASSOC
$sql = "select * from hockey where NUMBER=37";
try {  
  $db = open_db();
  $stmt = $db->query($sql);
  $result = $stmt->fetch(PDO::FETCH_ASSOC);
  if ($result["NUMBER"] != "37") {
     print("FAILED: select test5\n");
  }
  if ($result["NAME"] != "PATRICE BERGERON") {
     print("FAILED: select test5\n");
  }
  if ($result["POSITION"] != "Forward") {
     print("FAILED: select test5\n");
  }
  if ($result["TEAM"] != "Bruins") {
     print("FAILED: select test5\n");
  }
  $stmt = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  

// select test6 - query and fetch with FETCH_NUM
$sql = "select * from hockey where NUMBER=37";
try {  
  $db = open_db();
  $stmt = $db->query($sql);
  $result = $stmt->fetch(PDO::FETCH_NUM);
  if ($result[0] != "37") {
     print("FAILED: select test6\n");
  }
  if ($result[1] != "PATRICE BERGERON") {
     print("FAILED: select test6\n");
  }
  if ($result[2] != "Forward") {
     print("FAILED: select test6\n");
  }
  if ($result[3] != "Bruins") {
     print("FAILED: select test6\n");
  }
  $stmt = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  


// select test7 - query and fetch with FETCH_BOTH
$sql = "select * from hockey where NUMBER=37";
try {  
  $db = open_db();
  $stmt = $db->query($sql);
  $result = $stmt->fetch(PDO::FETCH_BOTH);
  if ($result[0] != "37") {
     print("FAILED: select test7\n");
  }
  if ($result[1] != "PATRICE BERGERON") {
     print("FAILED: select test7\n");
  }
  if ($result[2] != "Forward") {
     print("FAILED: select test7\n");
  }
  if ($result[3] != "Bruins") {
     print("FAILED: select test7\n");
  }
  if ($result["NUMBER"] != "37") {
     print("FAILED: select test7\n");
  }
  if ($result["NAME"] != "PATRICE BERGERON") {
     print("FAILED: select test7\n");
  }
  if ($result["POSITION"] != "Forward") {
     print("FAILED: select test7\n");
  }
  if ($result["TEAM"] != "Bruins") {
     print("FAILED: select test7\n");
  }
  $stmt = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  

// select test8 - query and fetch with FETCH_OBJ
$sql = "select * from hockey where NUMBER=37";
try {  
  $db = open_db();
  $stmt = $db->query($sql);
  $obj = $stmt->fetch(PDO::FETCH_OBJ);
  if ($obj->NUMBER != "37") {
     print("FAILED: select test8\n");
  }
  if ($obj->NAME != "PATRICE BERGERON") {
     print("FAILED: select test8\n");
  }
  if ($obj->POSITION != "Forward") {
     print("FAILED: select test8\n");
  }
  if ($obj->TEAM != "Bruins") {
     print("FAILED: select test8\n");
  }
  $stmt = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  

// select test9 - query and fetchObject
$sql = "select * from hockey where NUMBER=37";
class player {
  public $NUMBER;
  public $NAME;
  public $POSITION;
  public $TEAM;
}
try {  
  $db = open_db();
  $stmt = $db->query($sql);
  $obj = $stmt->fetchObject('player');
  if ($obj->NUMBER != "37") {
     print("FAILED: select test9\n");
  }
  if ($obj->NAME != "PATRICE BERGERON") {
     print("FAILED: select test9\n");
  }
  if ($obj->POSITION != "Forward") {
     print("FAILED: select test9\n");
  }
  if ($obj->TEAM != "Bruins") {
     print("FAILED: select test9\n");
  }
  $obj = NULL;
  $stmt = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  

// select test10 - failed query
$sql = "select no_such_column from hockey where NUMBER=37";
try {  
  $db = open_db();
  $stmt = $db->query($sql);
  // TODO: should db->errorCode() be able to get an error code?
  if ($stmt) {
     print("FAILED: select test10\n");  
  }
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  

// select test11 - query and fetch with missing column.
$sql = "select NAME from hockey where NUMBER=37";
try {  
  $db = open_db();
  $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING);
  foreach ($db->query($sql) as $row)
  {
     if ($row["NAME"] != "PATRICE BERGERON") {
        print("FAILED: select test11 - NAME\n");
     }
     if (isset($row["POSITION"])) {
     	print("FAILED: select test11 - POSITION\n");
     }
  }
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  


echo "done\n";
?>
