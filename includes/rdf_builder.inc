<?php

/**
 * @file
 * Build RDF like XML for OAC standard.
 *
 * Convenicence class to build RDF objects.
 *
 * @TODO: Conform to Drupal Coding Standards.
 */

class IslandoraRdfBuilder {

  public $dom_doc = NULL;

  function IslandoraRdfBuilder($dom_doc = NULL) {
    if (!isset($dom_doc)) {
      $this->dom_doc = new DOMDocument('1.0', 'utf-8');
      $this->dom_doc->formatOutput = TRUE;
      $this->root = $this->dom_doc->appendChild($this->dom_doc->createElement('rdf:RDF'));
      $this->root->setAttribute('xmlns:dc', "http://purl.org/dc/elements/1.1/");
      $this->root->setAttribute('xmlns:dcterms', "http://purl.org/dc/terms/");
      $this->root->setAttribute('xmlns:dms', "http://dms.stanford.edu/ns/");
      $this->root->setAttribute('xmlns:foaf', "http://xmlns.com/foaf/0.1/");
      $this->root->setAttribute('xmlns:ore', "http://www.openarchives.org/ore/terms/");
      $this->root->setAttribute('xmlns:rdf', "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
    }
    else {
      $this->dom_doc = $dom_doc;
      $this->root = $this->dom_doc->documentElement;
    }
  }

  /**
   * Returns value of IslandoraRdfBuilder object as an xml string
   * @return string
   */
  function printRDF() {
    return $this->dom_doc->saveXML();
  }

  /**
   * Add description node to existing RDF XML
   * @param string $about
   * the resource identified in the rdf 'about' attribute
   * @param array $children
   * A structured array of child elements to this node
   * Each array entry is a sub array with a manditory name, an optional array of attributes, and an optional value
   */
  function addDescriptionNode($about, $children = NULL) {
    $description = $this->dom_doc->createElement('rdf:Description');
    $description = $this->root->appendChild($description);
    $description->setAttribute("rdf:about", $about);

    foreach ($children as $child) {
      $new = $this->dom_doc->createElement($child['name']);
      $description->appendChild($new);
      if (isset($child['value'])) {
        $value = $this->dom_doc->createTextNode($child['value']);
        $new->appendChild($value);
      }
      if (array_key_exists('attributes', $child)) {
        foreach ($child['attributes'] as $key => $attVal) {
          $new->setAttribute($key, $attVal);
        }
      }
    }
  }

  /**
   * Takes a Nodelist of Description extracted from external RDF document and adds
   * them to this object's DOMDocument
   * @param NodeList $nodeList
   */
  function addDescriptionsFromNodelist($nodeList) {
    foreach ($nodeList as $domElement) {
      $dom_node = $this->dom_doc->importNode($domElement, TRUE);
      $this->root->appendChild($dom_node);
    }
  }

  /**
   * Takes a Description element extracted from external RDF document and adds
   * it to this object's DOMDocument
   * @param NodeList $nodeList
   */
  function addDescriptionElement($domElement) {
    $dom_node = $this->dom_doc->importNode($domElement, TRUE);
    $this->root->appendChild($dom_node);
  }

  function setXPath() {
    $this->xpath = new DOMXPath($this->dom_doc);
    $this->xpath->registerNamespace('rdf', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#');
    $this->xpath->registerNamespace('oa', 'http://www.w3.org/ns/openannotation/core/');
  }

  function deleteDescriptionElement($node_id) {
    $xpath = new DOMXPath($this->dom_doc);
    $candidates = $xpath->query("//rdf:Description[@rdf:about='$node_id']");
    foreach ($candidates as $candidate) {
      //$parent = $candidate->parentNode;
      // $parent->removeChild($candidate);
      $candidate->parentNode->removeChild($candidate);
    }
  }

  /**
   * Convenience method to structure input arrays for addDescriptionNode
   * @param string $name
   * Name of child tag
   * @param array $attributes
   * Array of key => value pairs (optional)
   * @param string $value
   * vale for tag (optional)
   * @return array
   */
  function build_child($name, $attributes = NULL, $value = NULL) {
    $return_array = array();
    $return_array['name'] = $name;
    if ($value) {
      $return_array['value'] = $value;
    }
    if ($attributes) {
      foreach ($attributes as $key => $attVal) {
        $return_array['attributes'][$key] = $attVal;
      }
    }
    return $return_array;
  }

  /**
   *
   * @param string $id
   *  the uuid containing urn:uuid:...
   * @return string
   *   a uuid containing urn:uuid:... this is the uuid of the hasBody node
   */
  function getAnnotationBodyUrn($id) {
    if (empty($this->xpath)) {
      $this->setXPath();
    }
    $candidates = $this->xpath->query('//rdf:Description[@rdf:about="' . $id . '"]/oa:hasBody');
    foreach ($candidates as $candidate) {
      $resource = $candidate->getAttribute('rdf:resource');
    }
    return $resource;
  }

  /**
   *
   * @param string $id
   *  the uuid containing urn:uuid:...
   * @return string
   *   a uuid containing urn:uuid:... this is the uuid of the hasTarget node
   *
   */
  function getAnnotationTargetUrn($id) {
    if (empty($this->xpath)) {
      $this->setXPath();
    }
    $candidates = $this->xpath->query('//rdf:Description[@rdf:about="' . $id . '"]/oa:hasTarget');
    foreach ($candidates as $candidate) {
      $resource = $candidate->getAttribute('rdf:resource');
    }
    return $resource;
  }

  /**
   *
   * @param string $id
   *  the uuid containing urn:uuid:...
   * @return string
   *   a uuid containing urn:uuid:... this is the uuid of the constrainedBy node
   *
   *
   */
  function getConstrainedTarget($id) {
    if (empty($this->xpath)) {
      $this->setXPath();
    }
    $candidates = $this->xpath->query('//rdf:Description[@rdf:about="' . $id . '"]/oa:constrainedBy');
    foreach ($candidates as $candidate) {
      $resource = $candidate->getAttribute('rdf:resource');
    }
    return $resource;
  }

}

