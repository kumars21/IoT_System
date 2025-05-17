$(function() {
    const $form = $("#network-form-wifi");
    const $modalSave = $("#network-modal-save");
    const $modalRevert = $("#network-modal-revert");

    $("#network-btn-save").on("click", function() {
        $form.addClass('was-validated');
        if (!$form.get(0).checkValidity())
            return false;
        new bootstrap.Modal($modalSave).show();
    });

    $("#network-btn-reset").on("click", function() {
        new bootstrap.Modal($modalRevert).show();
    });

    $modalSave.find(".btn-primary").on("click", function() {});

    $modalRevert.find(".btn-primary").on("click", function() {});
});
