<?php 

$db = NULL;  

function open_db() {
  $db = new PDO("nuodb:database=test@localhost;schema=Hockey", "cloud", "user") or die;
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

echo "done\n";
?>
